#include "jsB_timers.h"

static void setTimer(js_State *J)
{
	int handler = js_gettop(J) - 2;
	int timeValue = js_gettop(J) - 1;
	if (!js_iscallable(J, handler)) {
		js_typeerror(J, "expected callable as first argument");
	}
	if (js_isdefined(J, timeValue) && !js_isnumber(J, timeValue)) {
		js_typeerror(J, "expected timeout as number argument");
	}
	double timeout = js_checknumber(J, timeValue, 0);

	js_getregistry(J, "eventloop");
	if (js_isundefined(J, -1)) {
		js_error(J, "could not find eventloop");
	}
	js_getproperty(J, -1, "addHandler");
	js_rot2(J);
	/* props object */
	js_newobject(J);
	
	js_copy(J, handler);
	js_setproperty(J, -2, "handler");

	js_pushconst(J, "timer");
	js_setproperty(J, -2, "type");

	js_pushnumber(J, timeout);
	js_setproperty(J, -2, "delay");

	js_pushboolean(J, 0);
	js_setproperty(J, -2, "persistent");
}

/* XXX: add arguments support */
static void jsB_setTimeout(js_State *J)
{
	js_copy(J, 1);
	js_copy(J, 2);
	setTimer(J);

	js_newarray(J);
		/* copy arguments */
		for (int i = 3; i < js_gettop(J); ++i) {
			js_copy(J, i);
			js_setindex(J, -2, i - 2);
		}
		js_setlength(J, -1, js_gettop(J) -3);
	js_setproperty(J, -2, "arguments");

	js_call(J, 1);
}

/* XXX: add arguments support */
static void jsB_setInterval(js_State *J)
{
	js_copy(J, 1);
	js_copy(J, 2);
	setTimer(J);
	
	js_pushboolean(J, 1);
	js_setproperty(J, -2, "persistent");
	
	js_newarray(J);
		/* copy arguments */
		for (int i = 3; i < js_gettop(J); ++i) {
			js_copy(J, i);
			js_setindex(J, -2, i - 2);
		}
		js_setlength(J, -1, js_gettop(J) -3);
	js_setproperty(J, -2, "arguments");

	js_call(J, 1);
}

/* XXX: add arguments support */
static void jsB_setImmediate(js_State *J)
{	
	js_copy(J, 1);
	js_pushnumber(J, 0);
	setTimer(J);

	js_newarray(J);
		/* copy arguments */
		for (int i = 2; i < js_gettop(J); ++i) {
			js_copy(J, i);
			js_setindex(J, -2, i - 2);
		}
		js_setlength(J, -1, js_gettop(J) -2);
	js_setproperty(J, -2, "arguments");
	
	js_call(J, 1);
}

static void clearTimer(js_State *J)
{
	if (!js_isnumber(J, 1)) {
		js_typeerror(J, "expected timer id as number argument");
	}

	js_getregistry(J, "eventloop");
	if (js_isundefined(J, -1)) {
		js_error(J, "could not find eventloop");
	}
	js_getproperty(J, -1, "removeHandler");
	js_rot2(J);
	js_copy(J, 1);
	js_call(J, 1);
}

static void jsB_clearTimeout(js_State *J)
{
	clearTimer(J);
}

static void jsB_clearInterval(js_State *J)
{
	clearTimer(J);
}

static void jsB_clearImmediate(js_State *J)
{
	clearTimer(J);
}

void jsB_Timers(js_State *J)
{
	js_getregistry(J, "jsB_Timers");
	if (js_isdefined(J, -1)) {
		return;
	}
	js_pop(J, 1);

	js_newobject(J);
	js_newcfunction(J, jsB_setTimeout, "setTimeout", 2);
	js_defproperty(J, -2, "setTimeout", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_setInterval, "setInterval", 2);
	js_defproperty(J, -2, "setInterval", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_setImmediate, "setImmediate", 1);
	js_defproperty(J, -2, "setImmediate", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_clearTimeout, "clearTimeout", 1);
	js_defproperty(J, -2, "clearTimeout", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_clearInterval, "clearInterval", 1);
	js_defproperty(J, -2, "clearInterval", JS_READONLY | JS_DONTCONF);
	js_newcfunction(J, jsB_clearImmediate, "clearImmediate", 1);
	js_defproperty(J, -2, "clearImmediate", JS_READONLY | JS_DONTCONF);

	js_copy(J, -1);
	js_setregistry(J, "jsB_Timers");
}