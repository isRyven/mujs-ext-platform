#include <mujs/mujs.h>
#include "../jsB_readstream.h"
#include "../jsB_arraybuffer.h"
#include "../jsB_typedarray.h"
#include "../jsB_utils.h"
#include "../jsB_eventemitter.h"
#include "../jsB_eventloop.h"

void register_lib(js_State *J)
{
	jsB_TypedArray(J, TYPEDARRAY_UINT8);
	js_setglobal(J, "Uint8Array");
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_Utils(J);
	js_setglobal(J, "Utils");
	jsB_ReadStream(J);
	js_setglobal(J, "ReadStream");
	jsB_EventLoop(J);
	js_construct(J, 0);
	js_copy(J, -1);
	js_setglobal(J, "eventloop");
	js_setregistry(J, "eventloop");
}
