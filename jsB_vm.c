#include <string.h>

#include "jsB_vm.h"
#include "jsB_arraybuffer.h"

static void jsB_VM_compile(js_State *J)
{
	if (!js_isdefined(J, 1)) {
		js_typeerror(J, "Expected first argument to be set");
	}
	
	const char *filename = js_trystring(J, 2, "<script>");

	if (js_isundefined(J, 3)) {
		js_newobject(J);
	} else if (js_isdefined(J, 3) && !js_isobject(J, 3)) {
		js_typeerror(J, "Expected third argument to be an object");
	} else {
		js_copy(J, 3);
	}
	
	if (js_isstring(J, 1)) {
		const char* script = js_tostring(J, 1);
		js_loadstringE(J, filename, script);
	} else if (jsB_ArrayBuffer_instance(J, 1)) {
		js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
		char *script = (char*)js_malloc(J, backStore->length + 1);
		memcpy(script, backStore->data, backStore->length);
		script[backStore->length] = 0;
		if (js_try(J)) {
			js_free(J, script);
			js_throw(J);
		}
		js_loadstringE(J, filename, script);
		js_endtry(J);
		js_free(J, script);
	} else {
		js_typeerror(J, "Expected string or ArrayBuffer as first argument");
	}
}

void jsB_VM(js_State *J)
{
	js_getregistry(J, "jsB_VM");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	/* initialize it otherwise */
	js_pop(J, 1);
	js_newobject(J);
	js_newcfunction(J, jsB_VM_compile, "compile", 2);
	js_defproperty(J, -2, "compile", JS_READONLY | JS_DONTCONF);

	js_copy(J, -1);
	js_setregistry(J, "jsB_VM");
}