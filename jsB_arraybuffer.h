#ifndef JSB_ARRAYBUFFER_H
#define JSB_ARRAYBUFFER_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdlib.h>
#include <mujs/mujs.h>

typedef struct js_back_store_t {
	int length;
	char data[4];
} js_back_store_t;

/**
 * Returns ArrayBuffe constructor
 */
void jsB_ArrayBuffer(js_State *J);
/**
 * Validates ArrayBuffer instance 
 * @return true if valid ArrayBuffer instance
 */
int jsB_ArrayBuffer_instance(js_State *J, int idx);

/**
 * Get back store buffer
 */
js_back_store_t* jsB_ArrayBuffer_backstore(js_State *J, int idx);

#ifdef __cplusplus
}
#endif

#endif // JSB_ARRAYBUFFER_H
