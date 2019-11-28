#ifndef JSB_TYPEDARRAY_H
#define JSB_TYPEDARRAY_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdlib.h>
#include <mujs/mujs.h>

typedef enum {
	TYPEDARRAY_ANY,
	TYPEDARRAY_INT8,
	TYPEDARRAY_INT16,
	TYPEDARRAY_INT32,
	TYPEDARRAY_UINT8,
	TYPEDARRAY_UINT16,
	TYPEDARRAY_UINT32,
	TYPEDARRAY_FLOAT32,
	TYPEDARRAY_FLOAT64,
	TYPEDARRAY_BIGINT64,
	TYPEDARRAY_BIGUINT64,
	TYPEDARRAY_UINT8_CLAMPED
} jsB_TypedArray_elements_kind_t;

/**
 * Returns typed TypedArray constructor of a kind, or and object containing all typed arrays
 */
void jsB_TypedArray(js_State *J, jsB_TypedArray_elements_kind_t kind);
/**
 * Validates ArrayBuffer instance 
 * @return true if valid ArrayBuffer instance
 */
int jsB_TypedArray_instance(js_State *J, int idx, jsB_TypedArray_elements_kind_t kind);

#ifdef __cplusplus
}
#endif

#endif // JSB_TYPEDARRAY_H
