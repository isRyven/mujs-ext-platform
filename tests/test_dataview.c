#include <mujs/mujs.h>
#include "../jsB_arraybuffer.h"
#include "../jsB_dataview.h"

void register_lib(js_State *J)
{
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_DataView(J);
	js_setglobal(J, "DataView");	
}
