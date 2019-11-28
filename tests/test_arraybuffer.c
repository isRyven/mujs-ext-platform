#include <mujs/mujs.h>
#include "../jsB_arraybuffer.h"

void register_lib(js_State *J)
{
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
}
