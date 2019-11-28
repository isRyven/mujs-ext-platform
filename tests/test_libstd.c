#include <mujs/mujs.h>
#include "../jsB_arraybuffer.h"
#include "../jsB_typedarray.h"
#include "../jsB_libstd.h"

void register_lib(js_State *J)
{
	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_TypedArray(J, TYPEDARRAY_UINT8);
	js_setglobal(J, "Uint8Array");
	jsB_LibStd(J);
	js_setglobal(J, "std");
	js_pushconst(J, TEST_DIR);
	js_setglobal(J, "TEST_DIR");
}