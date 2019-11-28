#ifndef JSB_EVENTLOOP_H
#define JSB_EVENTLOOP_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <stdlib.h>
#include <mujs/mujs.h>

/**
 * Returns event loop constructor
 */
void jsB_EventLoop(js_State *J);

int jsB_EventLoop_instance(js_State *J, int idx);

#ifdef __cplusplus
}
#endif

#endif // JSB_EVENTLOOP_H
