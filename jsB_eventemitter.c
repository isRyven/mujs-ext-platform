#include "jsB_eventemitter.h"

#define U_NAME "EventEmitter"

static void jsB_new_EventEmitter(js_State *J)
{
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_newuserdata(J, U_NAME, NULL, NULL);
	js_newobject(J);
	js_setlocalregistry(J, -2, "listeners");
}

static void jsB_call_EventEmitter(js_State *J)
{
	js_typeerror(J, "expected new operator");
}

static void jsB_Subscription_cancel(js_State *J)
{
	if (!js_isobject(J, 0)) {
		js_typeerror(J, "expected this to be an object");
	}
	js_getproperty(J, 0, "ctx");
	if (!js_isuserdata(J, -1, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	js_getproperty(J, -1, "removeEventListener");
	js_rot2(J);		
	js_getproperty(J, 0, "event");
	if (!js_isstring(J, -1)) {
		js_typeerror(J, "expected event to be a string");
	}
	js_getproperty(J, 0, "handler");
	if (!js_iscallable(J, -1)) {
		js_typeerror(J, "expected handler to be callable");
	}
	js_call(J, 2);
	js_pushundefined(J);
}

static void jsB_EventEmitter_prototype_addEventListener(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "expected event name to be a string");
	}
	if (!js_iscallable(J, 2)) {
		js_typeerror(J, "expected event listener to be a function");
	}
	js_getlocalregistry(J, 0, "listeners");
	if (!js_hasproperty(J, -1, js_tostring(J, 1))) {
		js_newarray(J);
		js_copy(J, -1);
		js_setproperty(J, -3, js_tostring(J, 1));
	}
	int length = js_getlength(J, -1);
	js_copy(J, 2);
	js_setindex(J, -2, length);
	js_setlength(J, -1, length + 1);
	/* return subscription cancel function */
	js_newcfunction(J, jsB_Subscription_cancel, "cancel", 0);
	js_getproperty(J, -1, "bind");
	js_rot2(J);
	js_newobject(J);
	js_copy(J, 0);
	js_setproperty(J, -2, "ctx");
	js_copy(J, 1);
	js_setproperty(J, -2, "event");
	js_copy(J, 2);
	js_setproperty(J, -2, "handler");
	js_call(J, 1);
}

static void jsB_EventEmitter_prototype_removeEventListener(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "expected event name to be a string");
	}
	if (!js_iscallable(J, 2)) {
		js_typeerror(J, "expected event listener to be a function");
	}
	js_getlocalregistry(J, 0, "listeners");
	if (js_hasproperty(J, -1, js_tostring(J, 1))) {
		int length = js_getlength(J, -1);
		for (int i = 0; i < length; ++i) {
			js_getindex(J, -1, i);
			js_copy(J, 2);
			if (js_strictequal(J)) {
				js_getproperty(J, -3, "splice");
				js_copy(J, -4);
				js_pushnumber(J, i);
				js_pushnumber(J, 1);
				js_call(J, 2);
				break;
			}
			js_pop(J, 2);
		}
	}
	js_pushundefined(J);
}

static void jsB_EventEmitter_prototype_emit(js_State *J)
{
	if (!js_isuserdata(J, 0, U_NAME)) {
		js_typeerror(J, "invalid receiver");
	}
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "expected event name to be a string");
	}
	js_getlocalregistry(J, 0, "listeners");
	if (js_hasproperty(J, -1, js_tostring(J, 1))) {
		int length = js_getlength(J, -1);
		for (int i = 0; i < length; ++i) {
			js_getindex(J, -1, i);
			js_pushundefined(J);
			js_copy(J, 2);
			js_call(J, 1);
			js_pop(J, 1);
		}
	}
	js_pushundefined(J);
}

void jsB_EventEmitter(js_State *J)
{
	js_getregistry(J, "jsB_EventEmitter");
	if (js_isdefined(J, -1)) {
		return;
	}
	js_pop(J, 1);

	js_newobject(J);
	js_newcfunction(J, jsB_EventEmitter_prototype_addEventListener, "addEventListener", 2);
	js_setproperty(J, -2, "addEventListener");
	js_newcfunction(J, jsB_EventEmitter_prototype_removeEventListener, "removeEventListener", 2);
	js_setproperty(J, -2, "removeEventListener");
	js_newcfunction(J, jsB_EventEmitter_prototype_emit, "emit", 2);
	js_setproperty(J, -2, "emit");
	js_newcconstructor(J, jsB_call_EventEmitter, jsB_new_EventEmitter, "EventEmitter", 0);

	js_copy(J, -1);
	js_setregistry(J, "jsB_EventEmitter");
}