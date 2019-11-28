#include "jsB_arraybuffer.h"
#include <string.h>
#include <stddef.h>

#define M_MIN(a, b) (a < b ? a : b)
#define M_MAX(a, b) (a > b ? a : b)
#define M_CLAMP(v, a, b) M_MIN(M_MAX(v, a), b)

#define BUFFER_MAX 0x3fffffff
#define U_NAME "ArrayBuffer"
#define soffsetof(x,y) ((int)offsetof(x,y))

static int normalizeIndex(int i, int min, int max)
{
    i = i < 0 ? max + i : i;
    i = i < min ? min : (i > max ? (max - 1) : i);
    return i;
}

js_back_store_t nullStore = { 0 };

static void jsB_dealloc_ArrayBuffer(js_State *J, void *data);

/**
 * Allocates new array buffer instance
 */
static void jsB_new_ArrayBuffer(js_State *J)
{
	js_back_store_t *backStore = &nullStore;
	int length = js_checkinteger(J, 1, 0);
	if (length < 0) {
		js_rangeerror(J, "Invalid ArrayBuffer length: %i", length);
	}
	if (length > BUFFER_MAX) {
		js_rangeerror(J, "Out of bounds ArrayBuffer length: %i, max defined as %i", length, BUFFER_MAX);
	}
	if (length != 0) {
		int size = soffsetof(js_back_store_t, data) + length;
		backStore = (js_back_store_t*)js_malloc(J, size);
		memset(backStore, 0, size);
		backStore->length = length;
	}
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_newuserdatax(J, U_NAME, (void*)backStore, NULL, NULL, NULL, jsB_dealloc_ArrayBuffer);
	js_rot2pop1(J);
}

/**
 * Direct constructor call is forbidden
 */
static void jsB_call_ArrayBuffer(js_State *J)
{
	js_typeerror(J, "Constructor ArrayBuffer requires 'new'");
}

/**
 * Main ArrayBuffer deallocator
 */
static void jsB_dealloc_ArrayBuffer(js_State *J, void *data)
{
	if (data && data != &nullStore) {
		js_free(J, data);
	}
}

/**
 * byteLength setter
 */
static void jsB_ArrayBuffer_prototype_set_byteLength(js_State *J)
{
	js_pushundefined(J);
}

/**
 * byteLength getter, gets buffer length from backing store struct
 */
static void jsB_ArrayBuffer_prototype_get_byteLength(js_State *J)
{
	js_back_store_t *backStore;
	if (!jsB_ArrayBuffer_instance(J, 0)) {
        js_typeerror(J, "Method get ArrayBuffer.prototype.byteLength called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	backStore = (js_back_store_t*)js_touserdata(J, 0, U_NAME);
	js_pushnumber(J, backStore->length);
}

static void jsB_ArrayBuffer_prototype_slice(js_State *J)
{
	js_back_store_t *backStore, *newBackStore;
	int begin, end, length, newLength;
	if (!jsB_ArrayBuffer_instance(J, 0)) {
		js_typeerror(J, "Method ArrayBuffer.prototype.slice called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	backStore = (js_back_store_t*)js_touserdata(J, 0, U_NAME);
	length = backStore->length;
	begin = js_checkinteger(J, 1, 0);
	end = js_checkinteger(J, 2, length);
	/* handle negative offsets */
	begin = normalizeIndex(begin, 0, length);
	end = normalizeIndex(end, 0, length);
	/* clamp offsets */
	newLength = end - begin;
	/* instantiate new array buffer */
	jsB_ArrayBuffer(J);
	js_pushnumber(J, (double)newLength);
	js_construct(J, 1);
	/* copy values */
	newBackStore = (js_back_store_t*)js_touserdata(J, -1, U_NAME);
	if (newLength != 0 && newBackStore != &nullStore) {
		memcpy(newBackStore->data, backStore->data + begin, newLength);
	}
}

int jsB_ArrayBuffer_instance(js_State *J, int idx)
{
	return js_isuserdata(J, idx, U_NAME);
}

js_back_store_t* jsB_ArrayBuffer_backstore(js_State *J, int idx)
{
	return js_touserdata(J, idx, U_NAME);
}

/**
 * Returns ArrayBuffer constructor
 */
void jsB_ArrayBuffer(js_State *J) 
{
	js_getregistry(J, "jsB_ArrayBuffer");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	/* initialize it otherwise */
	js_pop(J, 1);
	/* prototype object */
	js_newobject(J); 
	js_newcfunction(J, jsB_ArrayBuffer_prototype_slice, "ArrayBuffer.prototype.slice", 2);
	js_defproperty(J, -2, "slice", JS_DONTCONF | JS_READONLY);
	js_newcfunction(J, jsB_ArrayBuffer_prototype_get_byteLength, "get ArrayBuffer.prototype.byteLength", 0);
    js_newcfunction(J, jsB_ArrayBuffer_prototype_set_byteLength, "set ArrayBuffer.prototype.byteLength", 1);
    js_defaccessor(J, -3, "byteLength", JS_DONTCONF | JS_READONLY);
    /* make prototype non-extensible */
	js_freeze(J);
	/* define constructor */
	js_newcconstructor(J, jsB_call_ArrayBuffer, jsB_new_ArrayBuffer, "ArrayBuffer", 1);
	/* cache constructor */
	js_copy(J, -1);
	js_setregistry(J, "jsB_ArrayBuffer");
}
