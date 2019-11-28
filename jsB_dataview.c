#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

#include "jsB_dataview.h"
#include "jsB_arraybuffer.h"

#define M_MIN(a, b) (a < b ? a : b)
#define M_MAX(a, b) (a > b ? a : b)
#define M_CLAMP(v, a, b) M_MIN(M_MAX(v, a), b)
#define M_IN_RANGEX(v, x, y) (v >= x && v <= y)

typedef union { 
	int i; 
	double d; 
	float f; 
	int64_t l; 
} floatint_t;

#define U_NAME "DataView"

typedef enum {
	NAN_ELEMENTS,
	UINT8_ELEMENTS,
	INT8_ELEMENTS,
	UINT16_ELEMENTS,
	INT16_ELEMENTS,
	UINT32_ELEMENTS,
	INT32_ELEMENTS,
	FLOAT32_ELEMENTS,
	FLOAT64_ELEMENTS
} jsb_DataView_elements_kind_t;

typedef jsb_DataView_elements_kind_t elements_kind_t;


static void jsB_dealloc_DataView(js_State *J, void *data);

/**
 * Creates new array buffer view
 */
static void jsB_new_DataView(js_State *J)
{
	int bufferLength, byteOffset, byteLength, maxByteLength;
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_typeerror(J, "First argument to DataView constructor must be an ArrayBuffer");
	}
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
	bufferLength = backStore->length;
	byteOffset = js_checkinteger(J, 2, 0);
	maxByteLength = (bufferLength - byteOffset);
	byteLength = js_checkinteger(J, 3, maxByteLength);
	/* validate offset arugment */
	if (byteOffset < 0 || byteOffset > bufferLength) {
		js_rangeerror(J, "Start offset %i is outside the bounds of the buffer", byteOffset);
	}
	/* validate length argument */
	if (!M_IN_RANGEX(byteLength, 0, maxByteLength)) {
		js_rangeerror(J, "Invalid DataView length %i", byteLength);
	}
	/* validate both */
	if ((byteOffset + byteLength) > bufferLength) {
		js_rangeerror(J, "start offset %i is outside the bounds of the buffer", byteOffset);
	}
	/* instantiate new object */
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_newuserdatax(J, U_NAME, NULL, NULL, NULL, NULL, jsB_dealloc_DataView);
	/* store properties in local registry */
	js_copy(J, 1);
	js_setlocalregistry(J, -2, "buffer");
	js_pushnumber(J, byteOffset);
	js_setlocalregistry(J, -2, "byteOffset");
	js_pushnumber(J, byteLength);
	js_setlocalregistry(J, -2, "byteLength");
	/* pop constructor */
	js_rot2pop1(J);
}

/**
 * Direct constructor call is forbidden
 */
static void jsB_call_DataView(js_State *J)
{
	js_typeerror(J, "constructor DataView requires 'new'");
}

/**
 * Main DataView deallocator
 */
static void jsB_dealloc_DataView(js_State *J, void *data) 
{
}

/**
 * ArrayBuffer instance setter
 */
static void jsB_DataView_prototype_set_buffer(js_State *J)
{
	js_pushundefined(J);
}

/**
 * ArrayBuffer instance getter
 */
static void jsB_DataView_prototype_get_buffer(js_State *J)
{
	if (!jsB_DataView_instance(J, 0)) {
        js_typeerror(J, "Method get DataView.prototype.buffer called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	js_getlocalregistry(J, 0, "buffer");
}

/**
 * byteOffset setter
 */
static void jsB_DataView_prototype_set_byteOffset(js_State *J)
{
	js_pushundefined(J);
}

/**
 * byteOffset getter
 */
static void jsB_DataView_prototype_get_byteOffset(js_State *J)
{
	if (!jsB_DataView_instance(J, 0)) {
        js_typeerror(J, "Method get DataView.prototype.byteOffset called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	js_getlocalregistry(J, 0, "byteOffset");
}

/**
 * byteLength setter
 */
static void jsB_DataView_prototype_set_byteLength(js_State *J)
{
	js_pushundefined(J);
}

/**
 * byteLength getter
 */
static void jsB_DataView_prototype_get_byteLength(js_State *J)
{
	if (!jsB_DataView_instance(J, 0)) {
        js_typeerror(J, "Method get DataView.prototype.byteLength called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	js_getlocalregistry(J, 0, "byteLength");
}

static const char* __DataView_get_getter_name_string(elements_kind_t kind)
{
	switch (kind)
	{
		case UINT8_ELEMENTS: return "DataView.prototype.getUint8";
		case INT8_ELEMENTS: return "DataView.prototype.getInt8";
		case UINT16_ELEMENTS: return "DataView.prototype.getUint16";
		case INT16_ELEMENTS: return "DataView.prototype.getInt16";
		case UINT32_ELEMENTS: return "DataView.prototype.getUint32";
		case INT32_ELEMENTS: return "DataView.prototype.getInt32";
		case FLOAT32_ELEMENTS: return "DataView.prototype.getFloat32";
		case FLOAT64_ELEMENTS: return "DataView.prototype.getFloat64";
		default: return "invalid";
	}
}

static const char* __DataView_get_setter_name_string(elements_kind_t kind)
{
	switch (kind)
	{
		case UINT8_ELEMENTS: return "DataView.prototype.setUint8";
		case INT8_ELEMENTS: return "DataView.prototype.setInt8";
		case UINT16_ELEMENTS: return "DataView.prototype.setUint16";
		case INT16_ELEMENTS: return "DataView.prototype.setInt16";
		case UINT32_ELEMENTS: return "DataView.prototype.setUint32";
		case INT32_ELEMENTS: return "DataView.prototype.setInt32";
		case FLOAT32_ELEMENTS: return "DataView.prototype.setFloat32";
		case FLOAT64_ELEMENTS: return "DataView.prototype.setFloat64";
		default: return "invalid";
	}
}

static int __DataView_get_element_size(elements_kind_t kind)
{
	switch (kind)
	{
		case UINT8_ELEMENTS: return sizeof(uint8_t);
		case INT8_ELEMENTS: return sizeof(int8_t);
		case UINT16_ELEMENTS: return sizeof(uint16_t);
		case INT16_ELEMENTS: return sizeof(int16_t);
		case UINT32_ELEMENTS: return sizeof(uint32_t);
		case INT32_ELEMENTS: return sizeof(int32_t);
		case FLOAT32_ELEMENTS: return sizeof(float);
		case FLOAT64_ELEMENTS: return sizeof(double);
		default: return 0;
	}
}

static js_back_store_t* __DataView_get_backstore(js_State *J, int idx)
{
	js_getlocalregistry(J, idx, "buffer");
	js_back_store_t* backStore = (js_back_store_t*)js_touserdata(J, -1, "ArrayBuffer");
	js_pop(J, 1);
	return backStore;
}

static int __DataView_get_backstore_offset(js_State *J, int idx)
{
	js_getlocalregistry(J, idx, "byteOffset");
	int offset = js_toint32(J, -1);
	js_pop(J, 1);
	return offset;
}

static int __DataView_get_backstore_length(js_State *J, int idx)
{
	js_getlocalregistry(J, idx, "byteLength");
	int length = js_toint32(J, -1);
	js_pop(J, 1);
	return length;
}

static void __DataView_validate(js_State *J, int idx, const char *method)
{
	if (!js_isuserdata(J, idx, U_NAME)) {
		js_typeerror(J, "method get %s called on incompatible receiver #<%s>", method, js_resolvetypename(J, idx));
	}
}	

#define STOREBYTE(dataPtr, offset, value) \
	*((uint8_t*)dataPtr + (offset)) = (uint8_t)(value & 0xFF);

static void __DataView_store_int8(js_State *J, void *bufferData, int bufferIndex, uint32_t value)
{
	STOREBYTE(bufferData, bufferIndex, value);
}

static void __DataView_store_int16(js_State *J, void *bufferData, int bufferIndex, uint32_t value, int isLE)
{
	uint32_t b0 = value & 0xFF;
	uint32_t b1 = (value >> 8) & 0xFF;
	if (isLE) 
	{
		STOREBYTE(bufferData, bufferIndex, b0);
		STOREBYTE(bufferData, bufferIndex + 1, b1);
	}
	else
	{
		STOREBYTE(bufferData, bufferIndex, b1);
		STOREBYTE(bufferData, bufferIndex + 1, b0);
	}
}

static void __DataView_store_int32(js_State *J, void *bufferData, int bufferIndex, uint32_t value, int isLE)
{
	uint32_t b0 = value & 0xFF;
	uint32_t b1 = (value >> 8) & 0xFF;
	uint32_t b2 = (value >> 16) & 0xFF;
	uint32_t b3 = (value >> 24);
	if (isLE) 
	{
		STOREBYTE(bufferData, bufferIndex, b0);
		STOREBYTE(bufferData, bufferIndex + 1, b1);
		STOREBYTE(bufferData, bufferIndex + 2, b2);
		STOREBYTE(bufferData, bufferIndex + 3, b3);
	}
	else
	{
		STOREBYTE(bufferData, bufferIndex, b3);
		STOREBYTE(bufferData, bufferIndex + 1, b2);
		STOREBYTE(bufferData, bufferIndex + 2, b1);
		STOREBYTE(bufferData, bufferIndex + 3, b0);
	}
}

static void __DataView_store_int64(js_State *J, void *bufferData, int bufferIndex, uint32_t lowWord, uint32_t highWord, int isLE)
{
	uint32_t b0 = lowWord & 0xFF;
	uint32_t b1 = (lowWord >> 8) & 0xFF;
	uint32_t b2 = (lowWord >> 16) & 0xFF;
	uint32_t b3 = (lowWord >> 24);

	uint32_t b4 = highWord & 0xFF; 
	uint32_t b5 = (highWord >> 8) & 0xFF;
	uint32_t b6 = (highWord >> 16) & 0xFF;
	uint32_t b7 = (highWord >> 24);

	if (isLE) 
	{
		STOREBYTE(bufferData, bufferIndex, b0);
		STOREBYTE(bufferData, bufferIndex + 1, b1);
		STOREBYTE(bufferData, bufferIndex + 2, b2);
		STOREBYTE(bufferData, bufferIndex + 3, b3);
		STOREBYTE(bufferData, bufferIndex + 4, b4);
		STOREBYTE(bufferData, bufferIndex + 5, b5);
		STOREBYTE(bufferData, bufferIndex + 6, b6);
		STOREBYTE(bufferData, bufferIndex + 7, b7);
	}
	else
	{
		STOREBYTE(bufferData, bufferIndex, b7);
		STOREBYTE(bufferData, bufferIndex + 1, b6);
		STOREBYTE(bufferData, bufferIndex + 2, b5);
		STOREBYTE(bufferData, bufferIndex + 3, b4);
		STOREBYTE(bufferData, bufferIndex + 4, b3);
		STOREBYTE(bufferData, bufferIndex + 5, b2);
		STOREBYTE(bufferData, bufferIndex + 6, b1);
		STOREBYTE(bufferData, bufferIndex + 7, b0);
	}
}

static void __DataView_set(js_State *J, elements_kind_t kind)
{
	__DataView_validate(J, 0, __DataView_get_setter_name_string(kind));
	js_back_store_t *backStore = __DataView_get_backstore(J, 0);
	void *bufferData = backStore->data;
	int bufferLength = backStore->length;
	int offset = js_checkinteger(J, 1, 0);
	double value = js_checknumber(J, 2, 0);	
	int isLE = js_tryboolean(J, 3, 0);
	if (!(bufferData && bufferLength)) {
		js_typeerror(J, "Method %s cannot set value into null ArrayBuffer", __DataView_get_setter_name_string(kind));
	}
	if (offset < 0) {
		js_rangeerror(J, "Offset is outside the bounds of the DataView");
	}
	int viewOffset = __DataView_get_backstore_offset(J, 0);
	int viewLength = __DataView_get_backstore_length(J, 0);
	int viewElementSize = __DataView_get_element_size(kind);
	if ((offset + viewElementSize) > viewLength) {
        js_rangeerror(J, "Offset is outside the bounds of the DataView");
	}
    int bufferIndex = viewOffset + offset;
    switch (kind) 
    {
    	case UINT8_ELEMENTS:
    	case INT8_ELEMENTS:
    		__DataView_store_int8(J, bufferData, bufferIndex, (uint32_t)value);
    		break;
    	case UINT16_ELEMENTS:
    	case INT16_ELEMENTS:
    		__DataView_store_int16(J, bufferData, bufferIndex, (uint32_t)value, isLE);
    		break;
    	case UINT32_ELEMENTS:
    	case INT32_ELEMENTS:
    		__DataView_store_int32(J, bufferData, bufferIndex, (uint32_t)value, isLE);
    		break;
    	case FLOAT32_ELEMENTS: 
    	{
    		floatint_t fi = { .f = (float)value };
    		__DataView_store_int32(J, bufferData, bufferIndex, (uint32_t)fi.i, isLE);
    		break;
    	}
    	case FLOAT64_ELEMENTS:
    	{
    		floatint_t fi = { .d = value };
    		uint32_t lowWord = (uint32_t)(fi.l & 0xFFFFFFFF); 
    		uint32_t highWord = (uint32_t)(fi.l >> 32);
    		__DataView_store_int64(J, bufferData, bufferIndex, lowWord, highWord, isLE);
    		break;
    	}
    	default:
    		break;
    }
    js_pushundefined(J);
}

void jsB_DataView_prototype_setUint8(js_State *J)
{
	__DataView_set(J, UINT8_ELEMENTS);
}

void jsB_DataView_prototype_setInt8(js_State *J)
{
	__DataView_set(J, INT8_ELEMENTS);
}

void jsB_DataView_prototype_setUint16(js_State *J)
{
	__DataView_set(J, UINT16_ELEMENTS);
}

void jsB_DataView_prototype_setInt16(js_State *J)
{
	__DataView_set(J, INT16_ELEMENTS);
}

void jsB_DataView_prototype_setUint32(js_State *J)
{
	__DataView_set(J, UINT32_ELEMENTS);
}

void jsB_DataView_prototype_setInt32(js_State *J)
{
	__DataView_set(J, INT32_ELEMENTS);
}

void jsB_DataView_prototype_setFloat32(js_State *J)
{
	__DataView_set(J, FLOAT32_ELEMENTS);
}

void jsB_DataView_prototype_setFloat64(js_State *J)
{
	__DataView_set(J, FLOAT64_ELEMENTS);
}

#define LOADINT8(dataPtr, offset) *((int8_t*)dataPtr + (offset))
#define LOADUINT8(dataPtr, offset) *((uint8_t*)dataPtr + (offset))

void __DataView_load_int8(js_State *J, void *bufferData, int bufferIndex, int isSigned)
{
	if (isSigned)
		js_pushnumber(J, (double)LOADINT8(bufferData, bufferIndex));
	else
		js_pushnumber(J, (double)(LOADINT8(bufferData, bufferIndex) & 0xFF));
}

void __DataView_load_int16(js_State *J, void *bufferData, int bufferIndex, int isLE, int isSigned)
{
	int32_t b0; 
	int32_t b1; 
	int32_t result;
	if (isLE)
	{
		b0 = LOADUINT8(bufferData, bufferIndex);
		b1 = LOADINT8(bufferData, bufferIndex + 1);
		result = (b1 << 8) + b0;
	}
	else
	{
		b0 = LOADINT8(bufferData, bufferIndex);
		b1 = LOADUINT8(bufferData, bufferIndex + 1);
		result = (b0 << 8) + b1;
	}
	if (isSigned)
		js_pushnumber(J, (double)result);
	else
		js_pushnumber(J, (double)(result & 0xFFFF)); // drop the sign
}

void __DataView_load_int32(js_State *J, void *bufferData, int bufferIndex, int isLE, elements_kind_t kind)
{
	uint32_t b0 = LOADUINT8(bufferData, bufferIndex); 
	uint32_t b1 = LOADUINT8(bufferData, bufferIndex + 1); 
	uint32_t b2 = LOADUINT8(bufferData, bufferIndex + 2); 
	uint32_t b3 = LOADUINT8(bufferData, bufferIndex + 3); 
	uint32_t result;
	if (isLE)
		result = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	else
		result = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
	switch (kind)
	{
		case INT32_ELEMENTS:
			js_pushnumber(J, (double)(int32_t)result);
			return;
		case UINT32_ELEMENTS:
			js_pushnumber(J, (double)result);
			return;
		case FLOAT32_ELEMENTS:
		{
			floatint_t fi;
			fi.i = (int32_t)result;
			js_pushnumber(J, (double)fi.f);
			return;
		}
		default:
			break;
	}
	js_pushundefined(J);
}

void __DataView_load_float64(js_State *J, void *bufferData, int bufferIndex, int isLE)
{
	uint32_t b0 = LOADUINT8(bufferData, bufferIndex);
	uint32_t b1 = LOADUINT8(bufferData, bufferIndex + 1);
	uint32_t b2 = LOADUINT8(bufferData, bufferIndex + 2);
	uint32_t b3 = LOADUINT8(bufferData, bufferIndex + 3);
	uint32_t b4 = LOADUINT8(bufferData, bufferIndex + 4);
	uint32_t b5 = LOADUINT8(bufferData, bufferIndex + 5);
	uint32_t b6 = LOADUINT8(bufferData, bufferIndex + 6);
	uint32_t b7 = LOADUINT8(bufferData, bufferIndex + 7);
	uint32_t lowWord;
	uint32_t highWord;
	if (isLE)
	{
		lowWord = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
		highWord = (b7 << 24) | (b6 << 16) | (b5 << 8) | b4; 
	}
	else
	{
		highWord = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
		lowWord = (b4 << 24) | (b5 << 16) | (b6 << 8) | b7;	
	}
	floatint_t fi = { .l = ((int64_t)lowWord | (int64_t)highWord << 32ll) };
	js_pushnumber(J, fi.d);
}

static void __DataView_get(js_State *J, elements_kind_t kind)
{
	__DataView_validate(J, 0, __DataView_get_getter_name_string(kind));
	js_back_store_t *backStore = __DataView_get_backstore(J, 0);
	void *bufferData = backStore->data;
	int bufferLength = backStore->length;
	int offset = js_tryinteger(J, 1, 0);	
	int isLE = js_tryboolean(J, 2, 0);
	if (!(bufferData && bufferLength))
		js_typeerror(J, "method %s cannot get value from null ArrayBuffer", __DataView_get_getter_name_string(kind));
	if (offset < 0)
		js_rangeerror(J, "offset is outside the bounds of the DataView");
	int viewOffset = __DataView_get_backstore_offset(J, 0);
	int viewLength = __DataView_get_backstore_length(J, 0);
	int viewElementSize = __DataView_get_element_size(kind);
	if (offset + viewElementSize > viewLength)
        js_rangeerror(J, "offset is outside the bounds of the DataView");
    int bufferIndex = viewOffset + offset;
    switch (kind)
    {
    	case UINT8_ELEMENTS:
    		__DataView_load_int8(J, bufferData, bufferIndex, 0);
    		return;
    	case INT8_ELEMENTS:
    		__DataView_load_int8(J, bufferData, bufferIndex, 1);
    		return;
    	case UINT16_ELEMENTS:
	    	__DataView_load_int16(J, bufferData, bufferIndex, isLE, 0);
    		return;
    	case INT16_ELEMENTS:
	    	__DataView_load_int16(J, bufferData, bufferIndex, isLE, 1);
    		return;
    	case INT32_ELEMENTS:
    		__DataView_load_int32(J, bufferData, bufferIndex, isLE, INT32_ELEMENTS);
    		return;
    	case UINT32_ELEMENTS:
	    	__DataView_load_int32(J, bufferData, bufferIndex, isLE, UINT32_ELEMENTS);
    		return;
    	case FLOAT32_ELEMENTS:
    		__DataView_load_int32(J, bufferData, bufferIndex, isLE, FLOAT32_ELEMENTS);
    		return;
    	case FLOAT64_ELEMENTS:
    		__DataView_load_float64(J, bufferData, bufferIndex, isLE);
    		return;
    	default:
    		break;
    }
    js_pushundefined(J);
}

void jsB_DataView_prototype_getUint8(js_State *J)
{
	__DataView_get(J, UINT8_ELEMENTS);
}

void jsB_DataView_prototype_getInt8(js_State *J)
{
	__DataView_get(J, INT8_ELEMENTS);
}

void jsB_DataView_prototype_getUint16(js_State *J)
{
	__DataView_get(J, UINT16_ELEMENTS);
}

void jsB_DataView_prototype_getInt16(js_State *J)
{
	__DataView_get(J, INT16_ELEMENTS);
}

void jsB_DataView_prototype_getUint32(js_State *J)
{
	__DataView_get(J, UINT32_ELEMENTS);
}

void jsB_DataView_prototype_getInt32(js_State *J)
{
	__DataView_get(J, INT32_ELEMENTS);
}

void jsB_DataView_prototype_getFloat32(js_State *J)
{
	__DataView_get(J, FLOAT32_ELEMENTS);
}

void jsB_DataView_prototype_getFloat64(js_State *J)
{
	__DataView_get(J, FLOAT64_ELEMENTS);
}

int jsB_DataView_instance(js_State *J, int idx)
{
	return js_isuserdata(J, idx, U_NAME);
}

void jsB_DataView(js_State *J)
{
	js_getregistry(J, "jsB_DataView");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	/* initialize it otherwise */
	js_pop(J, 1);
	/* prototype object */
	js_newobject(J); 
	/* byteLength accessor */
	js_newcfunction(J, jsB_DataView_prototype_get_byteLength, "get DataView.prototype.byteLength", 0);
    js_newcfunction(J, jsB_DataView_prototype_set_byteLength, "set DataView.prototype.byteLength", 1);
    js_defaccessor(J, -3, "byteLength", JS_DONTCONF | JS_READONLY);
   	/* byteOffset accessor */
	js_newcfunction(J, jsB_DataView_prototype_get_byteOffset, "get DataView.prototype.byteOffset", 0);
    js_newcfunction(J, jsB_DataView_prototype_set_byteOffset, "set DataView.prototype.byteOffset", 1);
    js_defaccessor(J, -3, "byteOffset", JS_DONTCONF | JS_READONLY);
    /* buffer accessor */
	js_newcfunction(J, jsB_DataView_prototype_get_buffer, "get DataView.prototype.buffer", 0);
    js_newcfunction(J, jsB_DataView_prototype_set_buffer, "set DataView.prototype.buffer", 1);
    js_defaccessor(J, -3, "buffer", JS_DONTCONF | JS_READONLY);
    /* set int 8 */
    js_newcfunction(J, jsB_DataView_prototype_setUint8, "DataView.prototype.setUint8", 3);
    js_defproperty(J, -2, "setUint8", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_setInt8, "DataView.prototype.setInt8", 3);
    js_defproperty(J, -2, "setInt8", JS_READONLY | JS_DONTCONF);
    /* set int 16 */
    js_newcfunction(J, jsB_DataView_prototype_setUint16, "DataView.prototype.setUint16", 3);
    js_defproperty(J, -2, "setUint16", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_setInt16, "DataView.prototype.setInt16", 3);
    js_defproperty(J, -2, "setInt16", JS_READONLY | JS_DONTCONF);
    /* set int 32 */
    js_newcfunction(J, jsB_DataView_prototype_setUint32, "DataView.prototype.setUint32", 3);
    js_defproperty(J, -2, "setUint32", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_setInt32, "DataView.prototype.setInt32", 3);
    js_defproperty(J, -2, "setInt32", JS_READONLY | JS_DONTCONF);
    /* set double */
    js_newcfunction(J, jsB_DataView_prototype_setFloat32, "DataView.prototype.setFloat32", 3);
    js_defproperty(J, -2, "setFloat32", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_DataView_prototype_setFloat64, "DataView.prototype.setFloat64", 3);
    js_defproperty(J, -2, "setFloat64", JS_READONLY | JS_DONTCONF);
	/* get int 8 */
    js_newcfunction(J, jsB_DataView_prototype_getUint8, "DataView.prototype.getUint8", 2);
    js_defproperty(J, -2, "getUint8", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_getInt8, "DataView.prototype.getInt8", 2);
    js_defproperty(J, -2, "getInt8", JS_READONLY | JS_DONTCONF);
    /* get int 16 */
    js_newcfunction(J, jsB_DataView_prototype_getUint16, "DataView.prototype.getUint16", 2);
    js_defproperty(J, -2, "getUint16", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_getInt16, "DataView.prototype.getInt16", 2);
    js_defproperty(J, -2, "getInt16", JS_READONLY | JS_DONTCONF);
    /* get int 32 */
    js_newcfunction(J, jsB_DataView_prototype_getUint32, "DataView.prototype.getUint32", 2);
    js_defproperty(J, -2, "getUint32", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_getInt32, "DataView.prototype.getInt32", 2);
    js_defproperty(J, -2, "getInt32", JS_READONLY | JS_DONTCONF);
    /* get double */
    js_newcfunction(J, jsB_DataView_prototype_getFloat32, "DataView.prototype.getFloat32", 2);
    js_defproperty(J, -2, "getFloat32", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_DataView_prototype_getFloat64, "DataView.prototype.getFloat64", 2);
    js_defproperty(J, -2, "getFloat64", JS_READONLY | JS_DONTCONF);
    /* make prototype non-extensible */
	js_freeze(J);
	/* define constructor */
	js_newcconstructor(J, jsB_call_DataView, jsB_new_DataView, "DataView", 3);
	/* cache constructor */
	js_copy(J, -1);
	js_setregistry(J, "jsB_DataView");
}
