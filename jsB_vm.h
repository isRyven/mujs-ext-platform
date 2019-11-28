#ifndef JSB_VM_H
#define JSB_VM_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdlib.h>
#include <mujs/mujs.h>

void jsB_VM(js_State *J);

#ifdef __cplusplus
}
#endif

#endif // JSB_VM_H
