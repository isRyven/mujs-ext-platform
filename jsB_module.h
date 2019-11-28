#ifndef JSB_MODULE_H
#define JSB_MODULE_H

#ifdef __cplusplus
  extern "C" {
#endif

#include "mujs/mujs.h"

void jsB_Module(js_State *J);
void jsB_Module_define(js_State *J, void *data);

#ifdef __cplusplus
}
#endif

#endif // JSB_MODULE_H
