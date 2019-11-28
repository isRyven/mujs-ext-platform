#include <mujs/mujs.h>
#include "../jsB_eventemitter.h"

void register_lib(js_State *J)
{
	jsB_EventEmitter(J);
	js_setglobal(J, "EventEmitter");
}
