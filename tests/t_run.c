#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mujs/mujs.h>
#include "t_utils.h"

extern void register_lib(js_State *J);

static void jsB_print(js_State *J)
{
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stdout);
	}
	js_pushundefined(J);
}

static void jsB_repr(js_State *J)
{
	js_tryrepr(J, 1, "cannot represent value");
}

int main(int argc, const char **argv)
{
	js_State *J;
	int size;
	char *buffer;
	J = js_newstate(NULL, NULL, 0);
	js_newcfunction(J, jsB_print, "print", 0);
	js_setglobal(J, "print");
	js_newcfunction(J, jsB_repr, "repr", 0);
	js_setglobal(J, "repr");
	register_lib(J);
	if (get_file_contents(TEST_PATH, &size, &buffer) != 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return 1;
	}
	if (size == 0) {
		return 0;
	}
	if (js_ploadstring(J, TEST_PATH, buffer)) {
		fprintf(stderr, "%s\n", js_tostring(J, -1));
		return 1;
	}
	js_pushundefined(J);
	if (js_pcall(J, 0)) {
		fprintf(stderr, "%s\n", js_tostring(J, -1));
		return 1;
	}
	js_freestate(J);
	free(buffer);
	return 0;
}
