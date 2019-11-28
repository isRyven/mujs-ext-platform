#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <qsortx/qsortx.h>
#include "jsB_typedarray.h"
#include "jsB_arraybuffer.h"

#include <stdio.h>

#define M_MIN(a, b) (a < b ? a : b)
#define M_MAX(a, b) (a > b ? a : b)
#define M_CLAMP(v, a, b) M_MIN(M_MAX(v, a), b)
#define M_IN_RANGE(v, x, y) (v >= x && v < y)
#define M_IN_RANGEX(v, x, y) (v >= x && v <= y)
#define M_FLT_EPSILON 1.19209290E-07
#define soffsetof(x,y) ((int)offsetof(x,y))

static int normalizeIndex(int i, int min, int max)
{
    i = i < 0 ? max + i : i;
    i = i < min ? min : (i > max ? (max - 1) : i);
    return i;
}

static int isalphastr(const char *str)
{
    int c = 0;
    if (str[c] == 0) {
        return 0;
    }
    while(str[c] && c < 100) {
        if (!isalpha(str[c]) && str[c] != '_') {
            return 0;
        }
        c++;
    }
    return 1;
}

#define U_NAME "TypedArray"

static const char *typedArrayNames[] = {
	"Null",
    "Int8Array",
    "Int16Array",
    "Int32Array",
    "Uint8Array",
    "Uint16Array",
    "Uint32Array",
    "Float32Array",
    "Float64Array",
    "BigInt64Array",
    "BigUint64Array",
    "Uint8ClampedArray"
};

typedef double (*jsB_TypedArray_element_read_t)(const void *data);
typedef void (*jsB_TypedArray_element_write_t)(void *data, double value);

typedef struct {
    char size;
	jsB_TypedArray_elements_kind_t kind;
    jsB_TypedArray_element_read_t read;
    jsB_TypedArray_element_write_t write;
} jsB_TypedArray_type_t;

static js_back_store_t* __TypedArray_get_backstore(js_State *J, int idx)
{
	js_back_store_t *backStore;
	js_getlocalregistry(J, idx, "buffer");
   	backStore = (js_back_store_t *)jsB_ArrayBuffer_backstore(J, -1);
   	js_pop(J, 1);
   	return backStore;
}

static int __TypedArray_get_byteOffset(js_State *J, int idx)
{
	int byteOffset;
	js_getlocalregistry(J, idx, "byteOffset");
    byteOffset = js_tointeger(J, -1);
    js_pop(J, 1);
    return byteOffset;
}

static int __TypedArray_get_length(js_State *J, int idx)
{
	int length;
	js_getlocalregistry(J, idx, "length");
    length = js_tointeger(J, -1);
    js_pop(J, 1);
    return length;
}

static int __TypedArray_get_constructor(js_State *J, int idx)
{
    const char *name;
    jsB_TypedArray_type_t *ta;
    if (!jsB_TypedArray_instance(J, idx, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Expected typed array object");
    }
 	ta = (jsB_TypedArray_type_t *)js_touserdata(J, idx, U_NAME);
 	name = typedArrayNames[ta->kind];
 	js_getregistry(J, "jsB_TypedArray");
 	js_getproperty(J, -1, name);
 	js_rot2pop1(J);
    return js_isdefined(J, -1);
}

static void jsB_TypedArrayIterator_prototype_next(js_State *J)
{
    if (!js_isuserdata(J, 0, "Array Iterator"))
        js_typeerror(J, "Method Array Iterator.prototype.next called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    js_getlocalregistry(J, 0, "array");
    jsB_TypedArray_type_t *ta = js_touserdata(J, -1, U_NAME);
    int length = __TypedArray_get_length(J, -1);
    int byteOffset = __TypedArray_get_byteOffset(J, -1);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, -1);
    js_getlocalregistry(J, 0, "index");
    int index = js_toint32(J, -1);
    js_pop(J, 2);
    /* finish */
    if (index >= length) {
        js_newobject(J);
        js_pushundefined(J);
        js_setproperty(J, -2, "value");
        js_pushboolean(J, 1);
        js_setproperty(J, -2, "done");
        return;
    }
    /* store incremented index */
    js_pushnumber(J, index + 1);
    js_setlocalregistry(J, 0, "index");
    /* get value */
    char *data = backStore->data + (byteOffset + index * ta->size);
    double value = ta->read(data);
    /* result */
    js_newobject(J);
    /* call callback */
    js_getlocalregistry(J, 0, "callback");
    js_pushundefined(J); /* this */
    js_pushnumber(J, value);
    js_pushnumber(J, (double)index);
    js_call(J, 2);
    /* set return value in result object */
    js_setproperty(J, -2, "value");
    js_pushboolean(J, 0);
    js_setproperty(J, -2, "done");
}

static void jsB_new_TypedArrayIterator(js_State *J)
{
    /* 1 -> TypedArray instance */
    /* 2 -> getter callback */
    if (!jsB_TypedArray_instance(J, 1, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Expected typed array instance");
    }
    if (!js_iscallable(J, 2)) {
        js_typeerror(J, "%s is not a function", js_tostring(J, 1));
    }
    /* get prototype or define one */
    js_getregistry(J, "jsB_TypedArrayIteratorPrototype");
    if (js_isundefined(J, -1)) {
        js_newobject(J);
        js_newcfunction(J, jsB_TypedArrayIterator_prototype_next, "Array Iterator.prototype.next", 0);
        js_setproperty(J, -2, "next");
        js_freeze(J); /* make prototype non extensible */
        js_copy(J, -1); /* cache in the registry */
        js_setregistry(J, "jsB_TypedArrayIteratorPrototype");
    }
    js_newuserdata(J, "Array Iterator", NULL, NULL);
    js_copy(J, 1);
    js_setlocalregistry(J, -2, "array");
    js_copy(J, 2);
    js_setlocalregistry(J, -2, "callback");
    js_pushnumber(J, 0);
    js_setlocalregistry(J, -2, "index");
}

static void jsB_dealloc_TypedArray(js_State *J, void *data);

static void jsB_new_TypedArray(js_State *J)
{
    js_typeerror(J, "Abstract class TypedArray not directly constructable");
}

static void jsB_call_TypedArray(js_State *J)
{
    js_typeerror(J, "Abstract class TypedArray not directly constructable");
}

static void jsB_dealloc_TypedArray(js_State *J, void *data)
{
}

/**
 * Makes new TypedArray from other Object / Array / TypedArray values
 */
static void jsB_TypedArray_from(js_State *J)
{
	int length;
    if (!js_isobject(J, 1)) {
        js_typeerror(J, "%s is not iterable", js_tostring(J, 1));
    }
    if (!js_iscallable(J, 0)) {
        js_typeerror(J, "%s is not a constructor", js_tostring(J, 0));
    }
    length = js_getlength(J, 1);
    js_copy(J, 0);
    js_pushnumber(J, (double)length);
    js_construct(J, 1);
    if (!jsB_TypedArray_instance(J, -1, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.from called on incompatible receiver #<%s>", js_resolvetypename(J, -1));
    }
    /* copy values */
    for (int i = 0; i < length; i++) {
        js_getindex(J, 1, i); /* from iterable */
        js_setindex(J, -2, i); /* to new TypedArray instance */
    }
}

/**
 * Makes new TypedArray from specified values
 */
static void jsB_TypedArray_of(js_State *J)
{
	int length;
	double value;
	jsB_TypedArray_type_t *ta;
	js_back_store_t* backStore;
    if (!js_iscallable(J, 0)) {
        js_typeerror(J, "%s is not a constructor", js_tostring(J, 0));
    }
    length = js_gettop(J) - 1; /* exclude target object */
    js_copy(J, 0);
    js_pushnumber(J, (double)length);
    js_construct(J, 1);
    if (!jsB_TypedArray_instance(J, -1, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.of called on incompatible receiver #<%s>", js_resolvetypename(J, -1));
    }
    ta = (jsB_TypedArray_type_t*)js_touserdata(J, -1, U_NAME);
    backStore = __TypedArray_get_backstore(J, -1);
    /* set values */
    for (int i = 0; i < length; i++) {
        value = js_tonumber(J, i + 1);
        ta->write(backStore->data + (ta->size * i), value);
    }
}

/**
 * Get value by index
 */
static int jsb_TypedArray_value_getter(js_State *J, void *data, const char *name)
{
    int index, length, byteOffset;
    js_back_store_t *backStore;
    jsB_TypedArray_type_t *ta;
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "TypedArray getter called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (isalphastr(name)) {
        return 0;
    }
    /* get props */
	byteOffset = __TypedArray_get_byteOffset(J, 0);
    length = __TypedArray_get_length(J, 0);
   	backStore = __TypedArray_get_backstore(J, 0);
    /* validate range */
    index = atoi(name);
    if (!M_IN_RANGE(index, 0, length)) {
        js_pushundefined(J);
        return 1;
    }
    ta = (jsB_TypedArray_type_t *)data;
    js_pushnumber(J, ta->read(backStore->data + (byteOffset + index * ta->size)));
    return 1;
}

/**
 * Set value on index
 */
static int jsb_TypedArray_value_setter(js_State *J, void *data, const char *name)
{
    double value;
   	int index, length, byteOffset;
    js_back_store_t *backStore;
    jsB_TypedArray_type_t *ta;
     if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "TypedArray getter called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (isalphastr(name)) {
        return 0;
    }
    /* get props */
    byteOffset = __TypedArray_get_byteOffset(J, 0);
    length = __TypedArray_get_length(J, 0);
   	backStore = __TypedArray_get_backstore(J, 0);
    /* validate range */
    index = atoi(name);
    if (!M_IN_RANGE(index, 0, length)) {
        return 1;
    }
    value = js_tonumber(J, 1);
    ta = (jsB_TypedArray_type_t *)data;
    ta->write(backStore->data + (byteOffset + index * ta->size), value);
    return 1;
}

/**
 * ArrayBuffer instance setter
 */
static void jsB_TypedArray_prototype_set_buffer(js_State *J)
{
 	js_pushundefined(J);
}

/**
 * ArrayBuffer instance getter
 */
static void jsB_TypedArray_prototype_get_buffer(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method get TypedArray.prototype.buffer called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_getlocalregistry(J, 0, "buffer");
}

/**
 * TypedArray length setter
 */
static void jsB_TypedArray_prototype_set_length(js_State *J)
{
	js_pushundefined(J);
}

/**
 * TypedArray length getter, returns element count
 */
static void jsB_TypedArray_prototype_get_length(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method get TypedArray.prototype.length called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_getlocalregistry(J, 0, "length");
}

/**
 * byteLength setter
 */
static void jsB_TypedArray_prototype_set_byteLength(js_State *J)
{
	js_pushundefined(J);
}

/**
 * byteLength getter
 */
static void jsB_TypedArray_prototype_get_byteLength(js_State *J)
{
   	if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method get TypedArray.prototype.byteLength called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
 	js_getlocalregistry(J, 0, "byteLength");   
}

/**
 * byteOffset setter
 */
static void jsB_TypedArray_prototype_set_byteOffset(js_State *J)
{
	js_pushundefined(J);
}

/**
 * byteOffset getter
 */
static void jsB_TypedArray_prototype_get_byteOffset(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method get TypedArray.prototype.byteOffset called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_getlocalregistry(J, 0, "byteOffset");
}

typedef int (foreach_callback_t)(js_State *J, double val, int i, int len, void **ctx);
typedef struct { int length; double data[1]; } value_storage_t;

static void __TypedArray_generic_forEach(js_State *J, const char *name, foreach_callback_t callback)
{
	js_back_store_t *backStore;
	jsB_TypedArray_type_t *ta;
	char *data;
	double value;
	void *ctx = NULL;
	int length, byteOffset;
	if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.%s called on incompatible receiver #<%s>", name, js_resolvetypename(J, 0));
    }
    if (!js_iscallable(J, 1)) {
        js_typeerror(J, "%s is not a function", js_tostring(J, 1));
    }
    if (js_isdefined(J, 2) && !js_isobject(J, 2)) {
        js_pushundefined(J);
        js_replace(J, 2);
    }
    ta = js_touserdata(J, 0, U_NAME);
    backStore = __TypedArray_get_backstore(J, 0);
    length = __TypedArray_get_length(J, 0);
    byteOffset = __TypedArray_get_byteOffset(J, 0);
    js_pushundefined(J); /* user callback context */
	for (int i = 0; i < length; ++i) {
        data = backStore->data + (byteOffset + ta->size * i);
        js_copy(J, 1); /* set fn */
        js_copy(J, 2); /* set this */
        value = ta->read(data);
        js_pushnumber(J, value);
        js_pushnumber(J, (double)i);
        js_copy(J, 0);
        js_call(J, 3);
        if (callback) {
        	if (callback(J, value, i, length, &ctx)) {
        		js_rot3pop2(J);
	    		return;
        	}
        	js_replace(J, -3); /* update callback context */
        }
        js_pop(J, 1); /* pop return value */
    }
}

static void jsB_TypedArray_prototype_forEach(js_State *J)
{
    __TypedArray_generic_forEach(J, "forEach", NULL);
}

static int __TypedArray_find_predicate(js_State *J, double val, int i, int len, void **ctx)
{
    if (js_toboolean(J, -1)) {
        js_pushnumber(J, val);
        return 1;
    }
    js_pushundefined(J);
    return 0;
}

static void jsB_TypedArray_prototype_find(js_State *J)
{
    __TypedArray_generic_forEach(J, "find", __TypedArray_find_predicate);
}

static int __TypedArray_findIndex_predicate(js_State *J, double val, int i, int len, void **ctx)
{
    if (js_toboolean(J, -1)) {
        js_pushnumber(J, i);
        return 1;
    }
    js_pushnumber(J, -1);
    return 0;
}

static void jsB_TypedArray_prototype_findIndex(js_State *J)
{
    __TypedArray_generic_forEach(J, "findIndex", __TypedArray_findIndex_predicate);
}

static int __TypedArray_filter_predicate(js_State *J, double val, int i, int len, void **ctx)
{
	int j;
    if (!*ctx && len > 0) {
        value_storage_t *newStorage = js_malloc(J, soffsetof(value_storage_t, data) + (len * sizeof(double)));
        newStorage->length = 0;
        *ctx = newStorage;
    }
    if (js_toboolean(J, -1)) {
        value_storage_t *storage = (value_storage_t*)*ctx;
        storage->data[storage->length] = val;
        ++storage->length;
    }
    /* last element */
    if ((i + 1 == len) && *ctx) {
    	jsB_TypedArray_type_t *ta;
    	js_back_store_t *backStore;
    	value_storage_t *storage = (value_storage_t*)*ctx;
    	/* init new typed array with the specified size */
    	__TypedArray_get_constructor(J, 0);
        js_pushnumber(J, (double)storage->length);
        js_construct(J, 1);
        ta = js_touserdata(J, -1, U_NAME);
        backStore = __TypedArray_get_backstore(J, -1);
        for (j = 0; j < storage->length; ++j) {
        	ta->write(backStore->data + (j * ta->size), storage->data[j]);
        }
        /* cleanup */
    	if (*ctx) {
	        js_free(J, *ctx);
    	}
        return 1;
    }
    js_pushundefined(J); /* user callback context is undefined */
    return 0;
}

static void jsB_TypedArray_prototype_filter(js_State *J)
{
    __TypedArray_generic_forEach(J, "filter", __TypedArray_filter_predicate);
}

static int __TypedArray_map_predicate(js_State *J, double val, int i, int len, void **ctx)
{
	int j;
    if (!*ctx && len > 0) {
        value_storage_t *newStorage = js_malloc(J, soffsetof(value_storage_t, data) + (len * sizeof(double)));
        newStorage->length = 0;
        *ctx = newStorage;
    }
    if (!*ctx) {
    	js_pushundefined(J);
    	return 0;
    } 
    value_storage_t *storage = (value_storage_t*)*ctx;
    storage->data[storage->length] = js_tonumber(J, -1);
    ++storage->length;
    /* last element */
    if ((i + 1 == len) && *ctx) {
    	jsB_TypedArray_type_t *ta;
    	js_back_store_t *backStore;
    	value_storage_t *storage = (value_storage_t*)*ctx;
    	/* init new typed array with the specified size */
    	__TypedArray_get_constructor(J, 0);
        js_pushnumber(J, (double)storage->length);
        js_construct(J, 1);
        ta = js_touserdata(J, -1, U_NAME);
        backStore = __TypedArray_get_backstore(J, -1);
        for (j = 0; j < storage->length; ++j) {
        	ta->write(backStore->data + (j * ta->size), storage->data[j]);
        }
        /* cleanup */
    	if (*ctx) {
	        js_free(J, *ctx);
    	}
        return 1;
    }
    js_pushundefined(J); /* user callback context is undefined */
    return 0;
}

static void jsB_TypedArray_prototype_map(js_State *J)
{
    __TypedArray_generic_forEach(J, "map", __TypedArray_map_predicate);
}

static int __TypedArray_every_predicate(js_State *J, double val, int i, int len, void **ctx)
{
    if (!js_toboolean(J, -1)) {
        js_pushboolean(J, 0);
        return 1;
    }
    js_pushboolean(J, 1);
    return 0;
}

static void jsB_TypedArray_prototype_every(js_State *J)
{
    __TypedArray_generic_forEach(J, "every", __TypedArray_every_predicate);
}

static int __TypedArray_some_predicate(js_State *J, double val, int i, int len, void **ctx)
{
    if (js_toboolean(J, -1)) {
        js_pushboolean(J, 1);
        return 1;
    }
    js_pushboolean(J, 0);
    return 0;
}

static void jsB_TypedArray_prototype_some(js_State *J)
{
    __TypedArray_generic_forEach(J, "some", __TypedArray_some_predicate);
}

static void jsB_TypedArray_prototype_subarray(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.subarray called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int begin = js_checkinteger(J, 1, 0);
    int end = js_checkinteger(J, 2, length);
    begin = normalizeIndex(begin, 0, length);
    end = normalizeIndex(end, 0, length);
    int size = M_CLAMP(end - begin, 0, length);
    __TypedArray_get_constructor(J, 0);
    js_getlocalregistry(J, 0, "buffer");
    js_pushnumber(J, (double)(byteOffset + begin * ta->size));
    js_pushnumber(J, (double)size);
    js_construct(J, 3);
}

static void jsB_TypedArray_prototype_fill(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.fill called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    double value = js_checknumber(J, 1, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int begin = js_checkinteger(J, 2, 0);
    int end = js_checkinteger(J, 3, length);
    char *data;
    begin = normalizeIndex(begin, 0, length);
    end = normalizeIndex(end, 0, length);
    for (int i = begin; i < end; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        ta->write(data, value);
    }
    js_copy(J, 0);
}

static void jsB_TypedArray_prototype_includes(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.includes called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    double needle = js_checknumber(J, 1, 0);
    int begin = js_checkinteger(J, 2, 0);
    char *data;
    begin = normalizeIndex(begin, 0, length);
    for (int i = begin; i < length; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        if (fabs(ta->read(data) - needle) < M_FLT_EPSILON) {
            js_pushboolean(J, 1);
            return;
        }
    }
    js_pushboolean(J, 0);
}

static void jsB_TypedArray_prototype_set(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.set called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (!js_isobject(J, 1)) {
        js_typeerror(J, "Expected array-like object");
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    int dstLength = __TypedArray_get_length(J, 0);
    int dstByteOffset = __TypedArray_get_byteOffset(J, 0);
    int srcLength = js_getlength(J, 1);
    int dstIndexOffset = js_checkinteger(J, 2, 0);
    dstIndexOffset = normalizeIndex(dstIndexOffset, 0, dstLength);
    if (dstIndexOffset + srcLength > dstLength) {
        js_rangeerror(J, "Source is too large");
    }
    if (jsB_TypedArray_instance(J, 1, TYPEDARRAY_ANY)) {
        jsB_TypedArray_type_t *srcTa = js_touserdata(J, 1, U_NAME);
        js_back_store_t *srcBackStore = __TypedArray_get_backstore(J, 1);
        int srcByteOffset = __TypedArray_get_byteOffset(J, 1);
        if (srcTa->kind == ta->kind) {
            /* of the same type */
            char *srcData = srcBackStore->data + srcByteOffset;
            char *dstData = backStore->data + dstByteOffset + (dstIndexOffset * ta->size);
            memmove(dstData, srcData, srcLength * srcTa->size);
        } else {
            /* copy data one by one */
            char *srcData;
            char *dstData;
            for (int i = 0; i < srcLength; ++i) {
                srcData = srcBackStore->data + srcByteOffset + (i * srcTa->size);
                dstData = backStore->data + dstByteOffset + ((dstIndexOffset + i) * ta->size);
                ta->write(dstData, srcTa->read(srcData));
            }
        }
    } else {
        /* index by index copy */
        for (int i = 0; i < srcLength; ++i) {
            js_getindex(J, 1, i); /* get src value */
            js_setindex(J, 0, dstIndexOffset + i); /* set src value int dst */
        }
    }
    js_pushundefined(J);
}

static void jsB_TypedArray_prototype_indexOf(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.indexOf called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    double needle = js_checknumber(J, 1, 0);
    int begin = js_checkinteger(J, 2, 0);
    char *data;
    begin = normalizeIndex(begin, 0, length);
    for (int i = begin; i < length; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        if (fabs(ta->read(data) - needle) < M_FLT_EPSILON) {
            js_pushnumber(J, i);
            return;
        }
    }
    js_pushnumber(J, -1);
}

static void jsB_TypedArray_prototype_lastIndexOf(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.lastIndexOf called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    double needle = js_checknumber(J, 1, 0);
    int begin = js_checkinteger(J, 2, 0);
    char *data;
    begin = normalizeIndex(begin, 0, length);
    for (int i = (length - 1); i >= begin; --i) {
        data = backStore->data + (byteOffset + i * ta->size);
        if (fabs(ta->read(data) - needle) < M_FLT_EPSILON) {
            js_pushnumber(J, i);
            return;
        }
    }
    js_pushnumber(J, -1);
}

static void jsB_TypedArray_prototype_join(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.join called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, U_NAME);
    js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    const char *div = js_isstring(J, 1) ? js_tostring(J, 1) : ",";
    int divLength = strlen(div);
    char *data, *output;
    const char *num = NULL;
    int numLength = 0;
    int outputLength = 0;
    int outputSize = 1024;
    output = (char *)js_malloc(J, outputSize);
    *output = 0;
    if (js_try(J)) {
        js_free(J, output);
        js_pushstring(J, "");
        return;
    }
    for (int i = 0; i < length; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        js_pushnumber(J, ta->read(data));
        num = js_tostring(J, -1); /* convert double to string */
        numLength = strlen(num);
        js_pop(J, 1);
        if ((outputLength + numLength + divLength) >= outputSize) {
            output = js_realloc(J, output, outputSize * 2);
            outputSize *= 2;
        }
        memcpy(output + outputLength, num, numLength);
        outputLength += numLength;
        if (i + 1 < length) {
            memcpy(output + outputLength, div, divLength);
            outputLength += divLength;
        }
        output[outputLength] = 0;
    }
    js_pushstringu(J, output, 0);
    js_endtry(J);
    js_free(J, output);
}

static void jsB_TypedArray_prototype_reverse(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.reverse called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    char *data, *endData;
    double value;
    int count = length / 2;
    for (int i = 0; i < count; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        endData = backStore->data + (byteOffset + ta->size * (length - i - 1));
        value = ta->read(data);
        ta->write(data, ta->read(endData));
        ta->write(endData, value);
    }
    js_copy(J, 0);
}

static void jsB_TypedArray_prototype_reduce(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.reduce called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (!js_iscallable(J, 1)) {
        js_typeerror(J, "%s is not a function", js_tostring(J, 1));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int i = 0;
    char *data;
    double value;
    if (js_isundefined(J, 2)) {
        if (length == 0) {
            js_typeerror(J, "Reduce of empty array with no initial value");
        } else {
            js_pushnumber(J, ta->read(backStore->data + byteOffset));
            js_replace(J, 2); /* pluck first element from the array and use that as init value */
            i = 1;
        }
    }
    js_copy(J, 2); /* initial value */
    for (; i < length; ++i) {
        data = backStore->data + (byteOffset + i * ta->size);
        value = ta->read(data);
        js_pushundefined(J); /* this */
        js_copy(J, 1); /* callback */
        js_swap(J, -3); /* callback this init/prev */
        js_pushnumber(J, value);
        js_pushnumber(J, (double)i);
        js_copy(J, 0);
        js_call(J, 4); /* use result in next loop or return it */
    }
}

static void jsB_TypedArray_prototype_reduceRight(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.reduceRight called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (!js_iscallable(J, 1)) {
        js_typeerror(J, "%s is not a function", js_tostring(J, 1));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int i = length - 1;
    char *data;
    double value;
    if (js_isundefined(J, 2)) {
        if (length == 0) {
            js_typeerror(J, "Reduce of empty array with no initial value");
        } else {
            data = backStore->data + (byteOffset + i * ta->size); 
            js_pushnumber(J, ta->read(data));
            js_replace(J, 2);
            i -= 1;
        }
    }
    js_copy(J, 2); /* initial value */
    for (; i >= 0; --i) {
        data = backStore->data + (byteOffset + i * ta->size);
        value = ta->read(data);
        js_pushundefined(J); /* this */
        js_copy(J, 1); /* callback */
        js_swap(J, -3); /* callback this init/prev value */
        js_pushnumber(J, value);
        js_pushnumber(J, (double)i);
        js_copy(J, 0);
        js_call(J, 4); /* use result in next loop or return it */
    }
}

static void jsB_TypedArray_prototype_slice(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.slice called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int begin = js_checkinteger(J, 1, 0);
    int end = js_checkinteger(J, 2, length);
    char *data;
    begin = normalizeIndex(begin, 0, length);
    end = normalizeIndex(end, 0, length);
    __TypedArray_get_constructor(J, 0);
    /* ake a new array buffer copy */
    js_getlocalregistry(J, 0, "buffer");
    js_getproperty(J, -1, "slice");
    js_rot2(J); /* slice, buffer */
    js_pushnumber(J, (double)(byteOffset + (ta->size * begin)));
    js_pushnumber(J, (double)(byteOffset + (ta->size * end)));
    js_call(J, 2);
    js_construct(J, 1);
}

static void jsB_TypedArray_prototype_copyWithin(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.slice called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    int target = js_checkinteger(J, 1, 0);
    int begin = js_checkinteger(J, 2, 0);
    int end = js_checkinteger(J, 3, length);
    target = normalizeIndex(target, 0, length);
    begin = normalizeIndex(begin, 0, length);
    end = normalizeIndex(end, 0, length);
    int maxLength = length - target;
    int size = M_CLAMP(end - begin, 0, maxLength) * ta->size;
    int byteStart = byteOffset + begin * ta->size;
    int byteTarget = byteOffset + target * ta->size;
    if (byteStart != byteTarget && size > 0) {
        js_back_store_t *backStore = __TypedArray_get_backstore(J, 0);
        memmove(backStore->data + byteTarget, backStore->data + byteStart, (size_t)size);
    }
    js_copy(J, 0);
}

void __TypedArray_callback_keys(js_State *J)
{
    js_copy(J, 2);  /* return index */
}

void jsB_TypedArray_prototype_keys(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.keys called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_pushundefined(J);
    js_copy(J, 0);
    js_newcfunction(J, __TypedArray_callback_keys, "TypedArray.prototype.keys", 2);
    js_callscoped2(J, jsB_new_TypedArrayIterator, 2);
}

void __TypedArray_callback_values(js_State *J)
{
    js_copy(J, 1);  /* return value */
}

void jsB_TypedArray_prototype_values(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.values called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_pushundefined(J);
    js_copy(J, 0);
    js_newcfunction(J, __TypedArray_callback_values, "TypedArray.prototype.values", 2);
    js_callscoped2(J, jsB_new_TypedArrayIterator, 2);
}

void __TypedArray_callback_entries(js_State *J)
{
    js_newarray(J);
    js_copy(J, 2); /* index */
    js_setindex(J, -2, 0);
    js_copy(J, 1); /* value */
    js_setindex(J, -2, 1);
    js_setlength(J, -1, 2);
}

void jsB_TypedArray_prototype_entries(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.entries called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    js_pushundefined(J);
    js_copy(J, 0);
    js_newcfunction(J, __TypedArray_callback_entries, "TypedArray.prototype.entries", 2);
    js_callscoped2(J, jsB_new_TypedArrayIterator, 2);
}

typedef struct {
    js_State *J;
    jsB_TypedArray_type_t *ta;
} jsB_TypedArray_sort_t;

static int __TypedArray_callback_sort(const void *A, const void *B, void *data)
{
    jsB_TypedArray_sort_t *ctx = (jsB_TypedArray_sort_t*)data;
    js_State *J = ctx->J;
    double a = ctx->ta->read(A);
    double b = ctx->ta->read(B);
    int c;
    /* call user callback */
    if (js_iscallable(J, 1)) {
        js_copy(J, 1);
        js_pushundefined(J);
        js_pushnumber(J, a);
        js_pushnumber(J, b);
        js_call(J, 2);
        c = js_toint32(J, -1);
        js_pop(J, 1);
    } else {
        c = (int)(a - b);
    }
    return c;
}

void jsB_TypedArray_prototype_sort(js_State *J)
{
    if (!jsB_TypedArray_instance(J, 0, TYPEDARRAY_ANY)) {
        js_typeerror(J, "Method TypedArray.prototype.sort called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
    }
    if (!(js_iscallable(J, 1) || js_isundefined(J, 1)))
        js_typeerror(J, "The comparison function must be either a function or undefined");
    jsB_TypedArray_type_t *ta = js_touserdata(J, 0, "TypedArray");
    js_back_store_t *backStore =  __TypedArray_get_backstore(J, 0);
    int length = __TypedArray_get_length(J, 0);
    int byteOffset = __TypedArray_get_byteOffset(J, 0);
    jsB_TypedArray_sort_t ctx = { J, ta };
    qsortx(backStore->data + byteOffset, (size_t)length, (size_t)ta->size, __TypedArray_callback_sort, &ctx);
    js_copy(J, 0);
}

/**
 * Generic constructor for the concrete typed arrays
 */
static void jsB_TypedArray_generic_constructor(js_State *J, const char *name, jsB_TypedArray_type_t *type)
{
    int length = 0;
    int byteOffset = 0; 
    if (jsB_ArrayBuffer_instance(J, 1)) {
    	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
        byteOffset = js_checkinteger(J, 2, 0);
        if (byteOffset < 0) {
            js_rangeerror(J, "Start offset of %s should be a positive number", name);
        }
        if (byteOffset % type->size) {
            js_rangeerror(J, "Start offset of %s should be a multiple of %i", name, type->size);
        }
        int chunkSize = backStore->length - byteOffset;
        if (chunkSize < 0) {
            js_rangeerror(J, "Start offset %i is outside the bounds of the buffer", byteOffset);
        }
        if (chunkSize % type->size) {
            js_rangeerror(J, "Byte length of %s should be a multiple of %i", name, type->size);
        }
        length = chunkSize / type->size;
        /* limit length */
        if (js_isnumber(J, 3)) {
            int arg = js_toint32(J, 3);
            if (arg < 0 || length < arg) {
                js_rangeerror(J, "Invalid typed array length: %i", arg);
            }
            length = arg;
        }   
    } else if (js_isobject(J, 1)) {
        js_currentfunction(J);
        js_getproperty(J, -1, "from");
        js_rot2(J);
        js_copy(J, 1);
        js_call(J, 1);
        return;
    } else {
    	/* instantiate ArrayBuffer and replace the first argument */
        length = js_checkinteger(J, 1, 0);
        jsB_ArrayBuffer(J);
        js_pushnumber(J, (double)length * type->size);
        js_construct(J, 1);
        js_replace(J, 1);
    }
    /* instantiate new object */
    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdatax(J, U_NAME, type, jsb_TypedArray_value_getter, jsb_TypedArray_value_setter, NULL, jsB_dealloc_TypedArray);
	/* store properties in local registry */
	js_copy(J, 1);
	js_setlocalregistry(J, -2, "buffer");
	js_pushnumber(J, byteOffset);
	js_setlocalregistry(J, -2, "byteOffset");
	js_pushnumber(J, length * type->size);
	js_setlocalregistry(J, -2, "byteLength");
		js_pushnumber(J, length);
	js_setlocalregistry(J, -2, "length");
	/* pop constructor */
	js_rot2pop1(J);
}

// TypedArray constructors

#define DEFTYPEDARRAYTYPE(name, type, kind) \
    static double name ## _read (const void *data) { \
        return (double)*(const type*)data; \
    } \
    static void name ## _write (void *data, double value) { \
        type *num = data; \
        *num = (type)value; \
    } \
    jsB_TypedArray_type_t name ## _type = { sizeof(type), kind, name ## _read, name ## _write };

#define DEFTYPEDARRAYACC(name, type) \
    static void jsB_new_ ## name (js_State *J) {\
        jsB_TypedArray_generic_constructor(J, #name, &name ## _type); \
    } \
    static void jsB_call_ ## name (js_State *J) { \
        js_typeerror(J, "Constructor " #name " requires 'new'"); \
    }

// typed array template
#define DEFTYPEDARRAY(name, type, kind) \
    DEFTYPEDARRAYTYPE(name, type, kind) \
    DEFTYPEDARRAYACC(name, type)

DEFTYPEDARRAY(Int8Array, int8_t, TYPEDARRAY_INT8)
DEFTYPEDARRAY(Int16Array, int16_t, TYPEDARRAY_INT16)
DEFTYPEDARRAY(Int32Array, int32_t, TYPEDARRAY_INT32)
DEFTYPEDARRAY(Uint8Array, uint8_t, TYPEDARRAY_UINT8)
DEFTYPEDARRAY(Uint16Array, uint16_t, TYPEDARRAY_UINT16)
DEFTYPEDARRAY(Uint32Array, uint32_t, TYPEDARRAY_UINT32)
DEFTYPEDARRAY(Float32Array, float, TYPEDARRAY_FLOAT32)
DEFTYPEDARRAY(Float64Array, double, TYPEDARRAY_FLOAT64)
DEFTYPEDARRAY(BigInt64Array, int64_t, TYPEDARRAY_BIGINT64)
DEFTYPEDARRAY(BigUint64Array, uint64_t, TYPEDARRAY_BIGUINT64)

// Clamped uint8 array
static double jsB_Uint8ClampedArray_read(const void *data)
{
    return (double)*(const uint8_t*)data;
}
static void jsB_Uint8ClampedArray_write(void *data, double value)
{
    value = value > 255 ? 255 : value < 0 ? 0 : value;
    uint8_t *num = data;
    *num = (uint8_t)value;
}
jsB_TypedArray_type_t Uint8ClampedArray_type = { 
	sizeof(uint8_t), 
	TYPEDARRAY_UINT8_CLAMPED, 
	jsB_Uint8ClampedArray_read, 
	jsB_Uint8ClampedArray_write
};
DEFTYPEDARRAYACC(Uint8ClampedArray, uint8_t)

/**
 * Returns constructor of specified type
 */
void jsB_TypedArray(js_State *J, jsB_TypedArray_elements_kind_t kind)
{
	const char *typedArrayName;
	js_getregistry(J, "jsB_TypedArray");
	/* check cached constructor */
	if (js_isundefined(J, -1)) {
		/* initialize it otherwise */
		js_pop(J, 1);
		/* namespace object */
		js_newobject(J);
		/* prototype object */
		js_newobject(J);
		/* buffer accessor */
		js_newcfunction(J, jsB_TypedArray_prototype_get_buffer, "get TypedArray.proptotype.buffer", 0);
	    js_pushnull(J);
	    js_defaccessor(J, -3, "buffer", 0);
		/* length accessor */
	    js_newcfunction(J, jsB_TypedArray_prototype_get_length, "get TypedArray.proptotype.length", 0);
	    js_pushnull(J);
	    js_defaccessor(J, -3, "length", 0);
		/* byteLength accessor */
	    js_newcfunction(J, jsB_TypedArray_prototype_get_byteLength, "get TypedArray.proptotype.byteLength", 0);
	    js_pushnull(J);
	    js_defaccessor(J, -3, "byteLength", 0);
		/* byteOffset accessor */
	    js_newcfunction(J, jsB_TypedArray_prototype_get_byteOffset, "get TypedArray.proptotype.byteOffset", 0);
	    js_pushnull(J);
	    js_defaccessor(J, -3, "byteOffset", 0);
	    /* TypedArray methods */
        js_newcfunction(J, jsB_TypedArray_prototype_forEach, "TypedArray.prototype.forEach", 2);
	    js_setproperty(J, -2, "forEach");
		js_newcfunction(J, jsB_TypedArray_prototype_find, "TypedArray.prototype.find", 2);
	    js_setproperty(J, -2, "find");
		js_newcfunction(J, jsB_TypedArray_prototype_findIndex, "TypedArray.prototype.findIndex", 2);
	    js_setproperty(J, -2, "findIndex");
	    js_newcfunction(J, jsB_TypedArray_prototype_filter, "TypedArray.prototype.filter", 2);
	    js_setproperty(J, -2, "filter");
	    js_newcfunction(J, jsB_TypedArray_prototype_map, "TypedArray.prototype.map", 2);
	    js_setproperty(J, -2, "map");
        js_newcfunction(J, jsB_TypedArray_prototype_every, "TypedArray.prototype.every", 2);
	    js_setproperty(J, -2, "every");
	    js_newcfunction(J, jsB_TypedArray_prototype_some, "TypedArray.prototype.some", 2);
	    js_setproperty(J, -2, "some");
        js_newcfunction(J, jsB_TypedArray_prototype_subarray, "TypedArray.prototype.subarray", 2);
        js_setproperty(J, -2, "subarray");
        js_newcfunction(J, jsB_TypedArray_prototype_fill, "TypedArray.prototype.fill", 3);
        js_setproperty(J, -2, "fill");
        js_newcfunction(J, jsB_TypedArray_prototype_includes, "TypedArray.prototype.includes", 2);
        js_setproperty(J, -2, "includes");
        js_newcfunction(J, jsB_TypedArray_prototype_set, "TypedArray.prototype.set", 2);
        js_setproperty(J, -2, "set");
        js_newcfunction(J, jsB_TypedArray_prototype_indexOf, "TypedArray.prototype.indexOf", 2);
        js_setproperty(J, -2, "indexOf");
        js_newcfunction(J, jsB_TypedArray_prototype_lastIndexOf, "TypedArray.prototype.lastIndexOf", 2);
        js_setproperty(J, -2, "lastIndexOf");
        js_newcfunction(J, jsB_TypedArray_prototype_join, "TypedArray.prototype.join", 1);
        js_setproperty(J, -2, "join");
        js_newcfunction(J, jsB_TypedArray_prototype_reverse, "TypedArray.prototype.reverse", 0);
        js_setproperty(J, -2, "reverse");
        js_newcfunction(J, jsB_TypedArray_prototype_reduce, "TypedArray.prototype.reduce", 2);
        js_setproperty(J, -2, "reduce");
        js_newcfunction(J, jsB_TypedArray_prototype_reduceRight, "TypedArray.prototype.reduceRight", 2);
        js_setproperty(J, -2, "reduceRight");
        js_newcfunction(J, jsB_TypedArray_prototype_slice, "TypedArray.prototype.slice", 2);
        js_setproperty(J, -2, "slice");
        js_newcfunction(J, jsB_TypedArray_prototype_copyWithin, "TypedArray.prototype.copyWithin", 3);
        js_setproperty(J, -2, "copyWithin");
        js_newcfunction(J, jsB_TypedArray_prototype_keys, "TypedArray.prototype.keys", 0);
        js_setproperty(J, -2, "keys");
        js_newcfunction(J, jsB_TypedArray_prototype_values, "TypedArray.prototype.values", 0);
        js_setproperty(J, -2, "values");
        js_newcfunction(J, jsB_TypedArray_prototype_entries, "TypedArray.prototype.entries", 0);
        js_setproperty(J, -2, "entries");
        js_newcfunction(J, jsB_TypedArray_prototype_sort, "TypedArray.prototype.sort", 1);
        js_setproperty(J, -2, "sort");
		/* make prototype non-extensible */
		js_freeze(J);
		/* define constructor */
	    js_newcconstructor(J, jsB_new_TypedArray, jsB_call_TypedArray, U_NAME, 0);
	    /* TypedArray static methods */
	    js_newcfunction(J, jsB_TypedArray_from, "TypedArray.from", 3);
	    js_setproperty(J, -2, "from");
	    js_newcfunction(J, jsB_TypedArray_of, "TypedArray.of", 0);
	    js_setproperty(J, -2, "of");
	    /* All typed object constructors inherit from TypedArray constructor */
		#define DEFTYPEDARRAYCON(size, name) \
			/* TypedArray prototype */ \
		    js_getproperty(J, -1, "prototype"); \
		    js_pushnumber(J, (double)size); \
		    js_defproperty(J, -2, "BYTES_PER_ELEMENT", JS_READONLY | JS_DONTCONF); \
		    /* intermediate prototype */ \
		    js_newobjectx(J); \
		    /* TypedArray constructor, a prototype of the concrete constructor */ \
		    js_copy(J, -2); \
		    js_newcconstructorx(J, jsB_call_ ## name, jsB_new_ ## name, #name, 3); \
		    /* fields of the concrete constructor */ \
			js_pushnumber(J, (double)size); \
		    js_defproperty(J, -2, "BYTES_PER_ELEMENT", JS_READONLY | JS_DONTCONF); \
		    js_pushstring(J, #name); \
		    js_defproperty(J, -2, "name", JS_READONLY | JS_DONTCONF); \
		    /* add to namespace object */ \
		    js_defproperty(J, -3, #name, JS_READONLY | JS_DONTCONF); 

		DEFTYPEDARRAYCON(sizeof(int8_t), Int8Array);
	    DEFTYPEDARRAYCON(sizeof(int16_t), Int16Array);
	    DEFTYPEDARRAYCON(sizeof(int32_t), Int32Array);
	    DEFTYPEDARRAYCON(sizeof(uint8_t), Uint8Array);
	    DEFTYPEDARRAYCON(sizeof(uint16_t), Uint16Array);
	    DEFTYPEDARRAYCON(sizeof(uint32_t), Uint32Array);
	    DEFTYPEDARRAYCON(sizeof(float), Float32Array);
	    DEFTYPEDARRAYCON(sizeof(double), Float64Array);
	    DEFTYPEDARRAYCON(sizeof(int64_t), BigInt64Array);
	    DEFTYPEDARRAYCON(sizeof(uint64_t), BigUint64Array);
	    DEFTYPEDARRAYCON(sizeof(uint8_t), Uint8ClampedArray);

		js_pop(J, 1); /* pop TypedArray constructor */ 
		js_copy(J, -1); /* copy namespace object */
		js_setregistry(J, "jsB_TypedArray"); /* cache it */;
	}
	js_getproperty(J, -1, typedArrayNames[kind]);
	js_rot2pop1(J);
}

/**
 * Check if object represents TypedArray instance.
 */
int jsB_TypedArray_instance(js_State *J, int idx, jsB_TypedArray_elements_kind_t kind)
{
	if (kind == TYPEDARRAY_ANY) {
		return js_isuserdata(J, idx, U_NAME);
	}
	jsB_TypedArray_type_t *ta = js_touserdata(J, idx, U_NAME);
	return ta->kind == kind;
}
