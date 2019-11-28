#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mujs/mujs.h>
#include "jsB_arraybuffer.h"
#include "jsB_typedarray.h"
#include "jsB_dataview.h"
#include "jsB_libstd.h"
#include "jsB_tcp.h"
#include "jsB_eventloop.h"
#include "jsB_eventemitter.h"
#include "jsB_vm.h"
#include "jsB_module.h"
#include "jsB_utils.h"
#include "jsB_timers.h"
#include "tests/t_utils.h"

extern const unsigned char server_js[];
extern const unsigned char http_js[];

/* accepts in directory path */
int main(int argc, const char **argv)
{
	js_State *J = js_newstate(NULL, NULL, 0);

	if (js_try(J)) {
		fprintf(stdout, "%s\n", js_tostring(J, -1));
		js_freestate(J);
		return 1;
	}

	jsB_EventLoop(J);
	js_construct(J, 0);
	int eventLoopId = js_gettop(J) - 1;
	js_copy(J, -1);
	js_setregistry(J, "eventloop");

	jsB_ArrayBuffer(J);
	js_setglobal(J, "ArrayBuffer");
	jsB_TypedArray(J, TYPEDARRAY_INT8);
	js_setglobal(J, "Int8Array");
	jsB_TypedArray(J, TYPEDARRAY_INT16);
	js_setglobal(J, "Int16Array");
	jsB_TypedArray(J, TYPEDARRAY_INT32);
	js_setglobal(J, "Int32Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT8);
	js_setglobal(J, "Uint8Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT16);
	js_setglobal(J, "Uint16Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT32);
	js_setglobal(J, "Uint32Array");
	jsB_TypedArray(J, TYPEDARRAY_FLOAT32);
	js_setglobal(J, "Float32Array");
	jsB_TypedArray(J, TYPEDARRAY_FLOAT64);
	js_setglobal(J, "Float64Array");
	jsB_TypedArray(J, TYPEDARRAY_BIGINT64);
	js_setglobal(J, "BigInt64Array");
	jsB_TypedArray(J, TYPEDARRAY_BIGUINT64);
	js_setglobal(J, "BigUint64Array");
	jsB_TypedArray(J, TYPEDARRAY_UINT8_CLAMPED);
	js_setglobal(J, "Uint8ClampedArray");
	jsB_DataView(J);
	js_setglobal(J, "DataView");
	
	js_pushglobal(J);
	js_setglobal(J, "globalThis");

	jsB_Timers(J);
	js_getproperty(J, -1, "setTimeout");
	js_setglobal(J, "setTimeout");
	js_getproperty(J, -1, "setInterval");
	js_setglobal(J, "setInterval");
	js_getproperty(J, -1, "setImmediate");
	js_setglobal(J, "setImmediate");
	js_getproperty(J, -1, "clearTimeout");
	js_setglobal(J, "clearTimeout");
	js_getproperty(J, -1, "clearInterval");
	js_setglobal(J, "clearInterval");
	js_getproperty(J, -1, "clearImmediate");
	js_setglobal(J, "clearImmediate");
	js_pop(J, 1);

	js_newarray(J);
	for (int i = 1; i < argc; ++i) {
		js_pushstring(J, argv[i]);
		js_setindex(J, -2, i - 1);
	}
	js_setlength(J, -1, (argc - 1));
	js_setglobal(J, "scriptArgs");

	/* module cache */
	js_newobject(J);
		js_pushundefined(J);
		jsB_Module(J);
		js_callscoped(J, jsB_Module_define, "module", 1);
		js_setproperty(J, -2, "module");

		js_pushundefined(J);
		jsB_LibStd(J);
		js_callscoped(J, jsB_Module_define, "std", 1);
		js_setproperty(J, -2, "std");

		js_pushundefined(J);
		jsB_Tcp(J);
		js_callscoped(J, jsB_Module_define, "tcp", 1);
		js_setproperty(J, -2, "tcp");

		js_pushundefined(J);
		jsB_VM(J);
		js_callscoped(J, jsB_Module_define, "vm", 1);
		js_setproperty(J, -2, "vm");

		js_pushundefined(J);
		jsB_Utils(J);
		js_callscoped(J, jsB_Module_define, "utils", 1);
		js_setproperty(J, -2, "utils");

		js_pushundefined(J);
		jsB_EventEmitter(J);
		js_callscoped(J, jsB_Module_define, "eventemitter", 1);
		js_setproperty(J, -2, "eventemitter");

	/* make http module */
	jsB_Module(J);
		js_pushconst(J, "http.js");
		js_newobject(J);
			js_copy(J, -4); /* copy common cache */
			js_setproperty(J, -2, "cache");
			js_pushboolean(J, 1);
			js_setproperty(J, -2, "loadPrivate");
	js_construct(J, 2);
	/* call the http.js module */
	js_getproperty(J, -1, "compile");
	js_copy(J, -2);
	js_pushconst(J, (const char*)http_js);
	js_call(J, 1);
	js_pushundefined(J);
	js_call(J, 0);
	js_pop(J, 1);
	/* store the resuled module in common cache */
	js_setproperty(J, -2, "http.js");

	/* make server.js module */
	jsB_Module(J);
		js_pushconst(J, "server.js");
		js_newobject(J);
			js_copy(J, -4); /* copy common cache */
			js_setproperty(J, -2, "cache");
			js_pushboolean(J, 1);
			js_setproperty(J, -2, "loadPrivate");
	js_construct(J, 2);
	/* call the server.js module */
	js_getproperty(J, -1, "compile");
	js_copy(J, -2);
	js_pushconst(J, (const char*)server_js);
	js_call(J, 1);
	js_pushundefined(J);
	js_call(J, 0);
	js_pop(J, 1);
	/* store the resulted module in the common cache */
	js_setproperty(J, -2, "server.js");
	/* pop cache */
	js_pop(J, 1);

	while (1) {
		js_getproperty(J, eventLoopId, "poll");
		js_copy(J, eventLoopId);
		js_pushnumber(J, -1);
		js_call(J, 1);
		int rv = js_tointeger(J, -1);
		js_pop(J, 1);
		if (rv == 0) {
			break;
		}
	}

	js_endtry(J);
	js_freestate(J);
	return 0;
}
