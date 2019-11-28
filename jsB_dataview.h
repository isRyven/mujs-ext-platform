#ifndef JSB_DATAVIEW_H
#define JSB_DATAVIEW_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdlib.h>
#include <mujs/mujs.h>

/**
 * Returns DataView constructor
 */
void jsB_DataView(js_State *J);
/**
 * Validates DataView instance 
 * @return true if valid DataView instance
 */
int jsB_DataView_instance(js_State *J, int idx);

#ifdef __cplusplus
}
#endif

#endif // JSB_DATAVIEW_H
