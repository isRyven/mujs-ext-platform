#include <string.h>
#include <mujs/mujs.h>
#include "../jsB_vm.h"
#include "../jsB_arraybuffer.h"

void jsB_allocString(js_State *J)
{
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "Expected string as first argument");
	}
	const char *str = js_tostring(J, 1);
	unsigned int strSize = js_getstrsize(J, 1);
	jsB_ArrayBuffer(J);
	js_pushnumber(J, (double)strSize);
	js_construct(J, 1);
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, -1);
	memcpy(backStore->data, str, strSize);
}

void register_lib(js_State *J)
{
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_VM(J);
	js_setglobal(J, "VM");
	js_newcfunction(J, jsB_allocString, "allocString", 1);
	js_setglobal(J, "allocString");
}