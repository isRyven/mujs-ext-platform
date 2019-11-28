#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <mujs/mujs.h>
#include "../jsB_eventloop.h"

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

static void jsB_sleep(js_State *J) {
	if (js_isundefined(J, 1)) {
		js_typeerror(J, "expected milliseconds as first parm");
	}
	double ms = js_tonumber(J, 1);
	if (ms < 0) {
		js_rangeerror(J, "milliseconds must not be a negative number");
	}
	msleep((long)ms);
	js_pushundefined(J);
}

void register_lib(js_State *J)
{
	jsB_EventLoop(J);
	js_setglobal(J, "EventLoop");
	js_newcfunction(J, jsB_sleep, "sleep", 1);
	js_setglobal(J, "sleep");
}
