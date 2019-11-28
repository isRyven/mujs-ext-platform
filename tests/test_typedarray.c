#include <mujs/mujs.h>
#include "../jsB_arraybuffer.h"
#include "../jsB_typedarray.h"

void register_lib(js_State *J)
{
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_TypedArray(J, TYPEDARRAY_INT8);
	js_setglobal(J, "Int8Array");
	jsB_TypedArray(J, TYPEDARRAY_INT16);
	js_setglobal(J, "Int16Array");
	jsB_TypedArray(J, TYPEDARRAY_INT32);
	js_setglobal(J, "Int32Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT8);
	js_setglobal(J, "Uint8Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT16);
	js_setglobal(J, "Uint16Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT32);
	js_setglobal(J, "Uint32Array");
	jsB_TypedArray(J, TYPEDARRAY_FLOAT32);
	js_setglobal(J, "Float32Array");
	jsB_TypedArray(J, TYPEDARRAY_FLOAT64);
	js_setglobal(J, "Float64Array");
	jsB_TypedArray(J, TYPEDARRAY_BIGINT64);
	js_setglobal(J, "BigInt64Array");
	jsB_TypedArray(J, TYPEDARRAY_BIGUINT64);
	js_setglobal(J, "BigUint64Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT8_CLAMPED);
	js_setglobal(J, "Uint8ClampedArray");
}
