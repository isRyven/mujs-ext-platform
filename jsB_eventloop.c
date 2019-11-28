#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "jsB_eventloop.h"

#define M_MIN(a, b) (a < b ? a : b)
#define M_MAX(a, b) (a > b ? a : b)

#define U_NAME "EventLoop"

static int msleep(long msec)
{
    struct timespec ts;
    int res;
    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
    return res;
}

typedef enum {
	EVENTLOOP_TIMER,
	EVENTLOOP_IO
} jsB_EventLoop_kind_t;

static double getNow(js_State *J)
{
	js_getglobal(J, "Date");
	js_getproperty(J, -1, "now");
	js_pushundefined(J);
	js_call(J, 0);
	double now = js_tonumber(J, -1);
	js_pop(J, 2);
	return now;
}

static int jsB_EventLoop_private_getLength(js_State *J, int idx)
{
	js_getproperty(J, idx, "length");
	int length = js_tointeger(J, -1);
	js_pop(J, 1); /* - length value */
	return length;
}

static void jsB_EventLoop_private_decreaseLength(js_State *J, int idx)
{
	idx = idx < 0 ? js_gettop(J) + idx : idx;
	int length = jsB_EventLoop_private_getLength(J, idx);
	js_pushnumber(J, (double)M_MAX((length - 1), 0)); /* update length */
	js_setproperty(J, idx, "length");
}

static void jsB_EventLoop_private_increaseLength(js_State *J, int idx)
{
	idx = idx < 0 ? js_gettop(J) + idx : idx;
	int length = jsB_EventLoop_private_getLength(J, idx);
	js_pushnumber(J, M_MAX((length + 1), 0)); /* update length */
	js_setproperty(J, idx, "length");
}

static void jsB_dealloc_EventLoop(js_State *J, void *data);

static void jsB_new_EventLoop(js_State *J)
{
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_newuserdatax(J, U_NAME, NULL, NULL, NULL, NULL, jsB_dealloc_EventLoop);
	/* set hidden properties */
	/* timerItems */
	js_newobject(J);
		js_pushnumber(J, 0);
		js_defproperty(J, -2, "length", JS_DONTENUM);
	js_setlocalregistry(J, -2, "timerHandlers");
	/* ioItems */
	js_newobject(J);
		js_pushnumber(J, 0);
		js_defproperty(J, -2, "length", JS_DONTENUM);
	js_setlocalregistry(J, -2, "ioHandlers");
	/* counter */
	js_pushnumber(J, 0);
	js_setlocalregistry(J, -2, "counter");
}

static void jsB_call_EventLoop(js_State *J)
{
	js_typeerror(J, "constructor EventLoop requires 'new'");
}

static void jsB_dealloc_EventLoop(js_State *J, void *data) 
{
}

static void jsB_EventLoop_prototype_addHandler(js_State *J)
{
	if (!jsB_EventLoop_instance(J, 0)) {
        js_typeerror(J, "Method get EventLoop.prototype.addHandler called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	if (!js_isobject(J, 1)) {
		js_typeerror(J, "argument must be type of object");
	}
	
	js_getproperty(J, -1, "type");
	if (js_isundefined(J, -1)) {
		js_typeerror(J, "expected timer type field to be set");
	}
	const char *type = js_tostring(J, -1);
	js_pop(J, 1);

	js_getproperty(J, 1, "handler");
	if (!js_iscallable(J, -1)) {
		js_rangeerror(J, "expected handler field to be function");
	}

	if (strcmp("timer", type) == 0) {
		/* get delay value */
		js_getproperty(J, 1, "delay");
		if (!js_isnumber(J, -1)) {
			js_typeerror(J, "expected delay field to be set");
		}
		double delay = js_tonumber(J, -1);
		js_pop(J, 1);
		if (delay < 0) {
			js_rangeerror(J, "expected delay to be positive number");
		}
		/* is persistent? */
		js_getproperty(J, 1, "persistent");
		int persistent = js_toboolean(J, -1);
		js_pop(J, 1);
		/* get counter */
		js_getlocalregistry(J, 0, "counter");
		double nextId = js_tonumber(J, -1) + 1;
		js_pop(J, 1);
		/* increment counter */
		js_pushnumber(J, nextId);
		js_setlocalregistry(J, 0, "counter");
		/*  */
		js_getlocalregistry(J, 0, "timerHandlers");
		/* new handler object */
		js_newobject(J);
			/* set id */
			js_pushnumber(J, nextId);
			js_setproperty(J, -2, "id");
			/* set delay */
			js_pushnumber(J, delay);
			js_setproperty(J, -2, "delay");
			/* set handler */
			js_copy(J, -3);
			js_setproperty(J, -2, "handler");
			/* set start */
			js_pushnumber(J, getNow(J));
			js_setproperty(J, -2, "time");
			/* set persistence */
			js_pushboolean(J, persistent);
			js_setproperty(J, -2, "persistent");
			/* set arguments */
			js_getproperty(J, 1, "arguments");
			js_setproperty(J, -2, "arguments");

		js_setindex(J, -2, nextId);

		jsB_EventLoop_private_increaseLength(J, -1);

		js_pop(J, 2); /* pop func, handlers */
		/* push back new id */
		js_pushnumber(J, nextId);
	} else if (strcmp("io", type) == 0) {
		/* is persistent? */
		js_getproperty(J, 1, "persistent");
		int persistent = js_toboolean(J, -1);
		js_pop(J, 1);
		/* get counter */
		js_getlocalregistry(J, 0, "counter");
		double nextId = js_tonumber(J, -1) + 1;
		js_pop(J, 1);
		/* increment counter */
		js_pushnumber(J, nextId);
		js_setlocalregistry(J, 0, "counter");
		/*  */
		js_getlocalregistry(J, 0, "ioHandlers");
		/* new handler object */
		js_newobject(J);
			/* set id */
			js_pushnumber(J, nextId);
			js_setproperty(J, -2, "id");
			/* set handler */
			js_copy(J, -3);
			js_setproperty(J, -2, "handler");
			/* set persistence */
			js_pushboolean(J, persistent);
			js_setproperty(J, -2, "persistent");
		js_setindex(J, -2, nextId);

		jsB_EventLoop_private_increaseLength(J, -1);

		js_pop(J, 2); /* pop func, handlers */
		/* push back new id */
		js_pushnumber(J, nextId);
	} else {
		js_typeerror(J, "unsupported timer type");
	}
}

static void jsB_EventLoop_prototype_removeHandler(js_State *J)
{
	if (!jsB_EventLoop_instance(J, 0)) {
        js_typeerror(J, "Method get EventLoop.prototype.removeHandler called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	if (!js_isnumber(J, 1)) {
		js_typeerror(J, "argument must be type of number");
	}
	
	double id = js_tonumber(J, 1);
	if (id < 0) {
		js_typeerror(J, "out of bounds id");
	}
	
	js_getlocalregistry(J, 0, "timerHandlers");
	if (!js_hasindex(J, -1, id)) {
		js_pop(J, 1); /* pop timerHandlers */
		js_getlocalregistry(J, 0, "ioHandlers");
		if (!js_hasindex(J, -1, id)) {
			js_pop(J, 1); /* pop ioHandlers */
			js_pushnumber(J, 0);
			return;
		}
	}
	js_pop(J, 1); /* pop handler */
		
	/* update length */
	js_getproperty(J, -1, "length");
	double length = js_tonumber(J, -1);
	js_pop(J, 1); /* - length value */
	js_pushnumber(J, M_MAX((length - 1), 0)); /* update length */
	js_setproperty(J, -2, "length");
	/* end update length */

	js_delindex(J, -1, id);
	js_pop(J, 1); /* pop handlers */
	js_pushnumber(J, 1);
}


static void jsB_EventLoop_private_runTimerHandler(js_State *J)
{
	js_getproperty(J, 1, "arguments");
	int args = js_gettop(J) - 1;
	js_getproperty(J, 1, "handler");
	js_pushundefined(J);
	if (js_isdefined(J, args)) {
		int length = js_getlength(J, args);
		for (int i = 0; i < length; ++i) {
			js_getindex(J, args, i);
		}
		js_call(J, length);
	} else {
		js_call(J, 0);
	}
	js_pop(J, 1);
	
	js_getproperty(J, 1, "persistent");
	int persistent = js_toboolean(J, -1);
	js_pop(J, 1);

	if (persistent) {
		js_pushnumber(J, getNow(J));
		js_setproperty(J, 1, "time");
	} else {
		js_getlocalregistry(J, 0, "timerHandlers");
		js_delproperty(J, -1, js_tostring(J, 2));
		jsB_EventLoop_private_decreaseLength(J, -1);	
		js_pop(J, 1);
	}

	js_pushundefined(J);
}

static void jsB_EventLoop_private_runIoHandler(js_State *J)
{
	js_getproperty(J, 1, "handler");
	js_pushundefined(J);
	js_copy(J, 3);
	js_call(J, 1);
	double rv = js_tonumber(J, -1);
	/* work is done */
	if (rv == 1) {
		js_getproperty(J, 1, "persistent");
		int persistent = js_toboolean(J, -1);
		js_pop(J, 1);

		if (!persistent) {
			js_getlocalregistry(J, 0, "ioHandlers");
			js_delproperty(J, -1, js_tostring(J, 2));
			jsB_EventLoop_private_decreaseLength(J, -1);	
			js_pop(J, 1);	
		}
	}
}

static void jsB_EventLoop_private_runHandlerCheck(js_State *J)
{
	double minDelay = js_tonumber(J, 1);
	const char *pendingTimer = NULL;
	const char *propname;
	double then = getNow(J);

	js_getlocalregistry(J, 0, "timerHandlers");
	int timersLength = jsB_EventLoop_private_getLength(J, -1);

	if (timersLength > 0) {
		js_pushiterator(J, -1, 1);
		while ((propname = js_nextiterator(J, -1))) {
			js_getproperty(J, -2, propname);
			js_getproperty(J, -1, "delay");
			double delay = js_tonumber(J, -1);
			js_pop(J, 1);

			js_getproperty(J, -1, "time");
			double baseTime = js_tonumber(J, -1);
			js_pop(J, 1);

			double timeLeft = (baseTime + delay) - then;
			if (timeLeft <= 0) {
				js_copy(J, 0); /* this */
				js_copy(J, -2);
				js_pushstring(J, propname);
				js_callscoped2(J, jsB_EventLoop_private_runTimerHandler, 2);
				js_pushnumber(J, 1);
				return;
			}
			if (timeLeft <= minDelay) {
				pendingTimer = propname;
				minDelay = timeLeft;
			}
			js_pop(J, 1); /* pop property */
		}
		js_pop(J, 1); /* pop iterator */ 
	}

	js_getlocalregistry(J, 0, "ioHandlers");
	int iosLength = jsB_EventLoop_private_getLength(J, -1);

	if (iosLength > 0) {
		double delayFrac = minDelay / iosLength;
		js_pushiterator(J, -1, 1);
		while ((propname = js_nextiterator(J, -1))) {
			js_getproperty(J, -2, propname);
			js_copy(J, 0); /* this */
			js_copy(J, -2);
			js_pushstring(J, propname);
			js_pushnumber(J, delayFrac);
			js_callscoped2(J, jsB_EventLoop_private_runIoHandler, 3);
			double rv = js_tonumber(J, -1);
			if (rv == 1) {
				js_pushnumber(J, 1);
				return;
			}
			js_pop(J, 2); /* pop rv, property */
		}
		js_pop(J, 1); /* pop iterator */
	}

	if (minDelay > 0) {
		double diff = (then + minDelay) - getNow(J);
		if (diff > 0) {
			msleep(diff);
		}
	}

	if (pendingTimer) {
		js_copy(J, 0); /* this */
		js_getproperty(J, -3, pendingTimer); /* get from timerhandlers */
		js_pushstring(J, pendingTimer);
		js_callscoped2(J, jsB_EventLoop_private_runTimerHandler, 2);
		js_pushnumber(J, 1);
		return;
	}

	js_pushnumber(J, 0);
}

/* return false if no tasks left to do */
static void jsB_EventLoop_prototype_poll(js_State *J)
{
	if (!jsB_EventLoop_instance(J, 0)) {
        js_typeerror(J, "Method get EventLoop.prototype.poll called on incompatible receiver #<%s>", js_resolvetypename(J, 0));
	}
	if (js_isundefined(J, 1)) {
		js_typeerror(J, "Expected timeout to be number");
	}

	js_getlocalregistry(J, 0, "timerHandlers");
	int timersLength = jsB_EventLoop_private_getLength(J, -1);
	js_getlocalregistry(J, 0, "ioHandlers");
	int iosLength = jsB_EventLoop_private_getLength(J, -1);

	if (timersLength == 0 && iosLength == 0) {
		js_pushnumber(J, 0);
		return;
	}

	double timeout = js_tonumber(J, 1);

	if (timeout >= 0) {
		js_copy(J, 0);
		js_copy(J, 1);
		js_callscoped2(J, jsB_EventLoop_private_runHandlerCheck, 1);
		js_pushnumber(J, 1);
		return;
	} else {
		while (1) {
			js_copy(J, 0);
			js_copy(J, 1);
			js_callscoped2(J, jsB_EventLoop_private_runHandlerCheck, 1);
			double rv = js_tonumber(J, -1);
			if (rv == 1) {
				return;
			}
			js_pop(J, 1); /* pop value */
		}
	}
}

void jsB_EventLoop(js_State *J)
{
	js_getregistry(J, "jsB_EventLoop");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	/* initialize it otherwise */
	js_pop(J, 1);
	/* prototype object */
	js_newobject(J); 
	js_newcfunction(J, jsB_EventLoop_prototype_addHandler, "EventLoop.prototype.addHandler", 1);
    js_defproperty(J, -2, "addHandler", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_EventLoop_prototype_removeHandler, "EventLoop.prototype.removeHandler", 1);
    js_defproperty(J, -2, "removeHandler", JS_READONLY | JS_DONTCONF);
    js_newcfunction(J, jsB_EventLoop_prototype_poll, "EventLoop.prototype.poll", 0);
    js_defproperty(J, -2, "poll", JS_READONLY | JS_DONTCONF);
    /* make prototype non-extensible */
	js_freeze(J);
	/* define constructor */
	js_newcconstructor(J, jsB_call_EventLoop, jsB_new_EventLoop, "EventLoop", 0);
	/* cache constructor */
	js_copy(J, -1);
	js_setregistry(J, "jsB_EventLoop");
}

int jsB_EventLoop_instance(js_State *J, int idx)
{
	return js_isuserdata(J, idx, U_NAME);
}
