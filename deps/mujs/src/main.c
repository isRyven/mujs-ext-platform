#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <mujs/mujs.h>

#if defined(WIN32) || defined(_WIN32) || defined __CYGWIN__
  #define PATH_SEPARATOR '\\' 
#else 
  #define PATH_SEPARATOR '/'
#endif
#define MAX_OSPATH 256
#define ISPATHSEP(X) ((X) == '\\' || (X) == '/' || (X) == PATH_SEPARATOR)

const char* getbasename(const char *path, int stripExt)
{
	static char base[MAX_OSPATH] = { 0 };
	int length = (int)strlen(path) - 1;
	// skip trailing slashes
	while (length > 0 && (ISPATHSEP(path[length]))) 
		length--;
	while (length > 0 && !(ISPATHSEP(path[length - 1])))
		length--;
	strncpy(base, &path[length], MAX_OSPATH);
	length = strlen(base) - 1;
	// strip trailing slashes
	while (length > 0 && (ISPATHSEP(base[length])))
		base[length--] = '\0';
	if (stripExt) {
		char *ext = strrchr(base, '.');
		*ext = 0;
	}
	return base;
}

static char *xoptarg; /* Global argument pointer. */
static int xoptind = 0; /* Global argv index. */
static int xgetopt(int argc, char *argv[], char *optstring)
{
	static char *scan = NULL; /* Private scan pointer. */

	char c;
	char *place;

	xoptarg = NULL;

	if (!scan || *scan == '\0') {
		if (xoptind == 0)
			xoptind++;

		if (xoptind >= argc || argv[xoptind][0] != '-' || argv[xoptind][1] == '\0')
			return EOF;
		if (argv[xoptind][1] == '-' && argv[xoptind][2] == '\0') {
			xoptind++;
			return EOF;
		}

		scan = argv[xoptind]+1;
		xoptind++;
	}

	c = *scan++;
	place = strchr(optstring, c);

	if (!place || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return '?';
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			xoptarg = scan;
			scan = NULL;
		} else if (xoptind < argc) {
			xoptarg = argv[xoptind];
			xoptind++;
		} else {
			fprintf(stderr, "%s: option requires argument -%c\n", argv[0], c);
			return ':';
		}
	}

	return c;
}

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
void using_history(void) { }
void add_history(const char *string) { }
void rl_bind_key(int key, void (*fun)(void)) { }
void rl_insert(void) { }
char *readline(const char *prompt)
{
	static char line[500], *p;
	int n;
	fputs(prompt, stdout);
	p = fgets(line, sizeof line, stdin);
	if (p) {
		n = strlen(line);
		if (n > 0 && line[n-1] == '\n')
			line[--n] = 0;
		p = malloc(n+1);
		memcpy(p, line, n+1);
		return p;
	}
	return NULL;
}
#endif

#define PS1 "> "

static void jsB_gc(js_State *J)
{
	int report = js_toboolean(J, 1);
	js_gc(J, report);
	js_pushundefined(J);
}

static void jsB_load(js_State *J)
{
	int i, n = js_gettop(J);
	for (i = 1; i < n; ++i) {
		js_loadfile(J, js_tostring(J, i));
		js_pushundefined(J);
		js_call(J, 0);
		js_pop(J, 1);
	}
	js_pushundefined(J);
}

static void jsB_compile(js_State *J)
{
	const char *source = js_tostring(J, 1);
	const char *filename = js_isdefined(J, 2) ? js_tostring(J, 2) : "[string]";
	js_loadstring(J, filename, source);
}

static void jsB_print(js_State *J)
{
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stdout);
	}
	putchar('\n');
	js_pushundefined(J);
}

static void jsB_write(js_State *J)
{
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) putchar(' ');
		fputs(s, stdout);
	}
	js_pushundefined(J);
}

static void jsB_read(js_State *J)
{
	const char *filename = js_tostring(J, 1);
	FILE *f;
	char *s;
	int n, t;

	f = fopen(filename, "rb");
	if (!f) {
		js_error(J, "cannot open file '%s': %s", filename, strerror(errno));
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		fclose(f);
		js_error(J, "cannot seek in file '%s': %s", filename, strerror(errno));
	}

	n = ftell(f);
	if (n < 0) {
		fclose(f);
		js_error(J, "cannot tell in file '%s': %s", filename, strerror(errno));
	}

	if (fseek(f, 0, SEEK_SET) < 0) {
		fclose(f);
		js_error(J, "cannot seek in file '%s': %s", filename, strerror(errno));
	}

	s = malloc(n + 1);
	if (!s) {
		fclose(f);
		js_error(J, "out of memory");
	}

	t = fread(s, 1, n, f);
	if (t != n) {
		free(s);
		fclose(f);
		js_error(J, "cannot read data from file '%s': %s", filename, strerror(errno));
	}
	s[n] = 0;

	js_pushstring(J, s);
	free(s);
	fclose(f);
}

static void jsB_readline(js_State *J)
{
	char *line = readline("");
	if (!line) {
		js_pushnull(J);
		return;
	}
	js_pushstring(J, line);
	if (*line)
		add_history(line);
	free(line);
}

static void jsB_quit(js_State *J)
{
	exit(js_tonumber(J, 1));
}

static void jsB_repr(js_State *J)
{
	js_repr(J, 1);
}

#ifndef JS_COMPILER_DISABLED
static const char *require_js =
	"function require(name) {\n"
	"var cache = require.cache;\n"
	"if (name in cache) return cache[name];\n"
	"var exports = {};\n"
	"cache[name] = exports;\n"
	"Function('exports', read(name+'.js'))(exports);\n"
	"return exports;\n"
	"}\n"
	"require.cache = Object.create(null);\n"
;
#endif

static int eval_print(js_State *J, const char *source)
{
	if (js_ploadstring(J, "[stdin]", source)) {
		fprintf(stderr, "%s\n", js_trystring(J, -1, "Error"));
		js_pop(J, 1);
		return 1;
	}
	js_pushundefined(J);
	if (js_pcall(J, 0)) {
		fprintf(stderr, "%s\n", js_trystring(J, -1, "Error"));
		js_pop(J, 1);
		return 1;
	}
	if (js_isdefined(J, -1)) {
		printf("%s\n", js_tryrepr(J, -1, "can't convert to string"));
	}
	js_pop(J, 1);
	return 0;
}

static char *read_stdin(void)
{
	int n = 0;
	int t = 512;
	char *s = NULL;

	for (;;) {
		char *ss = realloc(s, t);
		if (!ss) {
			free(s);
			fprintf(stderr, "cannot allocate storage for stdin contents\n");
			return NULL;
		}
		s = ss;
		n += fread(s + n, 1, t - n - 1, stdin);
		if (n < t - 1)
			break;
		t *= 2;
	}

	if (ferror(stdin)) {
		free(s);
		fprintf(stderr, "error reading stdin\n");
		return NULL;
	}

	s[n] = 0;
	return s;
}

static void usage(void)
{
	fprintf(stderr, "Usage: mujs [options] [script [scriptArgs*]]\n");
	fprintf(stderr, "\t-i: Enter interactive prompt after running code.\n");
	fprintf(stderr, "\t-s: Check strictness.\n");
	fprintf(stderr, "\t-e: Evaluate string.\n");
	fprintf(stderr, "\t-c: Precompile script.\n");
	fprintf(stderr, "\t-f: Load precompiled script.\n");
	fprintf(stderr, "\t-d: Strip debug info from precompiled script.\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *input;
	js_State *J;
	int status = 0;
	int strict = 0;
	int interactive = 0;
	int evalstr = 0;
	int precompile = 0;
	int loadprecompile = 0;
	int stripdebug = 0;
	int i, c;

	while ((c = xgetopt(argc, argv, "isecfd")) != -1) {
		switch (c) {
		default: usage(); break;
		case 'i': interactive = 1; break;
		case 's': strict = 1; break;
		case 'e': evalstr = 1; break;
		case 'c': precompile = 1; break;
		case 'f': loadprecompile = 1; break;
		case 'd': stripdebug = 1; break;
		}
	}

	J = js_newstate(NULL, NULL, strict ? JS_STRICT : 0);

	js_newcfunction(J, jsB_gc, "gc", 0);
	js_setglobal(J, "gc");

	js_newcfunction(J, jsB_load, "load", 1);
	js_setglobal(J, "load");

	js_newcfunction(J, jsB_compile, "compile", 2);
	js_setglobal(J, "compile");

	js_newcfunction(J, jsB_print, "print", 0);
	js_setglobal(J, "print");

	js_newcfunction(J, jsB_write, "write", 0);
	js_setglobal(J, "write");

	js_newcfunction(J, jsB_read, "read", 1);
	js_setglobal(J, "read");

	js_newcfunction(J, jsB_readline, "readline", 0);
	js_setglobal(J, "readline");

	js_newcfunction(J, jsB_repr, "repr", 0);
	js_setglobal(J, "repr");

	js_newcfunction(J, jsB_quit, "quit", 1);
	js_setglobal(J, "quit");
#ifndef JS_COMPILER_DISABLED
	js_dostring(J, require_js);
#endif
	if (xoptind == argc) {
		interactive = 1;
	} else {
		c = xoptind++;
		js_newarray(J);
		// push executable path
		js_pushstring(J, argv[0]);
		js_setindex(J, -2, 0);
		i = 1;
		while (xoptind < argc) {
			js_pushstring(J, argv[xoptind++]);
			js_setindex(J, -2, i++);
		}
		js_setglobal(J, "scriptArgs");
		if (evalstr) {
			if (eval_print(J, argv[c]))
				status = 1;
		} else if (precompile) {
			for (i = c; i < argc; i++) {
				if (!js_ploadfile(J, argv[i])) {
					char *buf;
					int size = js_dumpscript(J, -1, &buf, stripdebug ? JS_BINSTRIPDEBUG : 0);
					if (buf) {
						char outpath[MAX_OSPATH] = { 0 };
						sprintf(outpath, "./%s.jsbin", getbasename(argv[i], 1));
						FILE *fd = fopen(outpath, "wb");
						if (!fd) {
							perror(argv[i]);
							status = 1;
							break;
						}
						int writes = fwrite(buf, 1, size, fd);
						if (writes != size) {
							perror(argv[i]);
							printf("could not write out buffer\n");
							status = 1;
						}
						fclose(fd);
						js_free(J, buf);
					} else {
						printf("could not precompile script\n");
						status = 1;
						break;
					}
				} else {
					printf("%s\n", js_tostring(J, -1));
					status = 1;
					break;
				}
			}
		} else if (loadprecompile) {
			if (!js_ploadbinfile(J, argv[c])) {
				js_pushundefined(J);
				if (js_pcall(J, 0)) {
					fprintf(stderr, "%s\n", js_tostring(J, -1));
					status = 1;
				}
			} else {
				fprintf(stderr, "%s\n", js_tostring(J, -1));
				status = 1;
			}
		} else if (js_dofile(J, argv[c]))
			status = 1;
	}

	if (interactive) {
		if (isatty(0)) {
			using_history();
			rl_bind_key('\t', rl_insert);
			input = readline(PS1);
			while (input) {
				eval_print(J, input);
				if (*input)
					add_history(input);
				free(input);
				input = readline(PS1);
			}
			putchar('\n');
		} else {
			input = read_stdin();
			if (!input || !js_dostring(J, input))
				status = 1;
			free(input);
		}
	}

	js_gc(J, 0);
	js_freestate(J);

	return status;
}
