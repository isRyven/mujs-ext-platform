#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "jsB_module.h"
#include "tests/t_utils.h"

#if defined(_WIN32) || defined __CYGWIN__
  	#define PATH_SEPARATOR '\\'
	#define IS_ASBOLUTE_PATH(path) \
		(path && strlen(path) >= 3 && \
			isalpha(path[0]) && path[1] == ':' && path[2] == '\\')
#else 
  	#define PATH_SEPARATOR '/'
	#define IS_ASBOLUTE_PATH(path) (path && path[0] && path[0] == '/')
#endif
#define MAX_OSPATH 256

#define ISPATHSEP(X) ((X) == '\\' || (X) == '/' || (X) == PATH_SEPARATOR)

static const char* u_path_dir(const char *path)
{
	static char dirpath[MAX_OSPATH] = { 0 };
	dirpath[0] = 0;
	int index = 0;
	int lastSepPos = 0;
	while (index < MAX_OSPATH && path[index])
	{
		if (ISPATHSEP(path[index])) 
			lastSepPos = index;
		index++;
	}
	strncpy(dirpath, path, lastSepPos);
	dirpath[lastSepPos] = 0;
	return dirpath;
}

static const char* u_path_base(const char *path)
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
	while (length > 0 && (ISPATHSEP(path[length])))
		base[length--] = '\0';
	return base;
}

static const char* u_path_push(char* path, const char* segment)
{
	int length = strlen(path);
	if (length > 0)
		path[length++] = '/';
	strncpy(path + length, segment, MAX_OSPATH - length);
	return path;
}

static const char* u_path_pop(char *path)
{
	int length = strlen(path) - 1;
	while (length > 0 && (ISPATHSEP(path[length]))) 
		length--; // trailing separators
	while (length > 0 && !(ISPATHSEP(path[length]))) 
		length--; // last known separator
	while (length > 0 && (ISPATHSEP(path[length]))) 
		length--; // remove trailing separators
	path[length + 1] = 0;
	return path;
}

static const char* u_path_resolve(const char *wd, const char *path)
{
	static char finalPath[MAX_OSPATH] = { 0 };
	static char tempSegment[MAX_OSPATH] = { 0 };
	finalPath[0] = 0;
	if (!wd) return path;
	strncpy(finalPath, wd, MAX_OSPATH);
	int i = 0, c = 0;
	while (1) 
	{
		// clear temp string
		tempSegment[0] = 0;
		while (ISPATHSEP(path[i])) 
			i++;
		if (path[i] == 0 || i >= MAX_OSPATH)
			break;
		for (c = 0; path[i] && i < MAX_OSPATH && !ISPATHSEP(path[i]); c++, i++)
			tempSegment[c] = path[i];
		tempSegment[c] = 0;
		if (strcmp(tempSegment, "..") == 0)
			u_path_pop(finalPath);
		else if (strcmp(tempSegment, ".") == 0)
			continue;
		else if (tempSegment[0])
			u_path_push(finalPath, tempSegment);
	}
	return finalPath;
}

static const char* u_path_join(const char *p1, const char *p2)
{
	static char finalPath[MAX_OSPATH] = { 0 };
	strcpy(finalPath, p1);
	u_path_push(finalPath, p2);
	return finalPath;
}

static int u_str_startsWith(const char *str, const char *fragment) 
{
	if (!str) return 0; // not string
	if (!fragment) return 0; // no fragment
	if (!str[0]) return 0; // empty string
	if (!fragment[0]) return 1; // empty fragment
	int length = strlen(fragment);
	return (strncmp(str, fragment, length) == 0);
}

static int u_str_endsWith(const char *str, const char *fragment) 
{
	if (!str) return 0; // not string
	if (!fragment) return 0; // no fragment
	if (!str[0]) return 0; // empty string
	if (!fragment[0]) return 1; // empty fragment
	int strLen = strlen(str);
	int fragmentLen = strlen(fragment);
	if (strLen < fragmentLen) return 0;
	return (strncmp(str + strLen - fragmentLen, fragment, fragmentLen) == 0);
}

static int __getContents(const char *path, char **buff)
{
	int size = 0;
	get_file_contents(path, &size, buff);
	return size;
}

// to from
static void __assign(js_State *J, void *data)
{
	if (!js_isobject(J, 1))
		js_typeerror(J, "expeceted object as first argument");
	if (!js_isobject(J, 2))
		js_typeerror(J, "expeceted object as second argument");
	js_pushiterator(J, 2, 1);
	const char *key;
	while ((key = js_nextiterator(J, -1)))
	{
		js_getproperty(J, 2, key);
		js_setproperty(J, 1, key);
	}
	js_pop(J, 1);
	js_copy(J, 1);
}

static void jsB_new_Module(js_State *J)
{
	if (!js_isstring(J, 1))
		js_typeerror(J, "Expected module path");
	js_currentfunction(J);
	js_getproperty(J, -1, "prototype");
	js_rot2pop1(J);
	js_newobjectx(J); // this
	js_copy(J, 1);
	js_setproperty(J, -2, "path");
	js_pushstring(J, u_path_dir(js_tostring(J, 1)));
		js_setproperty(J, -2, "dirpath");
	js_newobject(J);
		js_setproperty(J, -2, "exports");
	js_newobject(J);
		js_setproperty(J, -2, "cache");
	js_pushboolean(J, 0);
		js_setproperty(J, -2, "private");
	js_pushboolean(J, 1);
		js_setproperty(J, -2, "loadPrivate");
	js_newobject(J);
		js_setproperty(J, -2, "props");
	js_newarray(J);
		js_setproperty(J, -2, "paths");
	if (js_isobject(J, 2))
	{
		js_pushundefined(J);
		js_rot2(J);
		js_copy(J, 2);
		js_callscoped(J, __assign, NULL, 2);
	}
}

static void jsB_call_Module(js_State *J)
{
	js_typeerror(J, "constructor Module requires 'new'");
}

static void jsB_Module_prototype_compile(js_State *J)
{
	if (js_isundefined(J, 0) || !js_isobject(J, 0))
		js_typeerror(J, "invalid receiver");
	if (!js_isstring(J, 1))
		js_typeerror(J, "invalid source argument");
	js_pushundefined(J);
	// define scope variables
	js_newobject(J);
	js_getproperty(J, 0, "props");
	js_callscoped(J, __assign, NULL, 2);
	// add basic properties	
	js_getproperty(J, 0, "exports");
	js_setproperty(J, -2, "exports");
	js_copy(J, 0);
	js_setproperty(J, -2, "module");
	js_getprototypeof(J, 0);
	js_getproperty(J, -1, "require");
	js_rot2pop1(J);
	if (!js_iscallable(J, -1))
		js_typeerror(J, "invalid prototype");
	// bind context
	js_getproperty(J, -1, "bind");
		js_rot2(J);
		js_copy(J, 0); // this
		js_call(J, 1);
	js_setproperty(J, -2, "require");
	js_getproperty(J, 0, "path");
	js_setproperty(J, -2, "__filename");
	js_getproperty(J, 0, "dirpath");
	js_setproperty(J, -2, "__dirname");
	// get module path
	js_getproperty(J, 0, "path");
	const char *moduleName = js_tostring(J, -1);
	js_pop(J, 1);
	js_loadstringE(J, moduleName, js_tostring(J, 1));
}

static int __require_from_cache(js_State *J, const char *moduleName) 
{
	js_getproperty(J, 0, "cache");
	// load from cache
	if (js_hasproperty(J, -1, moduleName))
	{
		js_getproperty(J, -1, "private");
		js_getproperty(J, 0, "loadPrivate");
		int isPrivate = js_toboolean(J, -2);
		int loadPrivate = js_toboolean(J, -1); js_pop(J, 2);
		if (isPrivate && !loadPrivate)
			js_typeerror(J, "cannot load private modules");
		js_getproperty(J, -1, "exports");
		js_rot3pop2(J);
		return 1;
	}
	js_pop(J, 1);
	return 0;
}

static int __require_new_module(js_State *J, const char *path)
{
	char *code;
	if (__getContents(path, &code) == -1)
		return 0;
	// construct new module
	jsB_Module(J);
	js_pushstring(J, path);
	js_newobject(J);
		js_getproperty(J, 0, "cache");
			js_setproperty(J, -2, "cache");
		js_getproperty(J, 0, "private");
			js_setproperty(J, -2, "private");
		js_getproperty(J, 0, "loadPrivate");
			js_setproperty(J, -2, "loadPrivate");
		js_getproperty(J, 0, "props");
			js_setproperty(J, -2, "props");
		js_getproperty(J, 0, "paths");
			js_setproperty(J, -2, "paths");
	js_construct(J, 2);
	// load code
	if (u_str_endsWith(path, ".json")) 
	{
		js_getglobal(J, "JSON");
		js_getproperty(J, -1, "parse");
		js_rot2(J);
		js_pushconst(J, code);
		js_call(J, 1);
		js_setproperty(J, -2, "exports");
	}
	else
	{
		js_getproperty(J, -1, "compile");
		js_copy(J, -2);
		js_pushconst(J, code);
		js_call(J, 1);
		js_pushundefined(J);
		js_call(J, 0);
		js_pop(J, 1);
	}
	return 1;
}

static void jsB_Module_prototype_require(js_State *J)
{
	if (js_isundefined(J, 0) || !js_isobject(J, 0))
		js_typeerror(J, "invalid receiver");
	if (!js_isstring(J, 1))
		js_typeerror(J, "invalid module name argument");
	if (js_getlength(J, 1) == 0)
		js_typeerror(J, "empty module name is not allowed");
	const char *moduleName = js_tostring(J, 1);
	// relative path
	if (u_str_startsWith(moduleName, "./") || u_str_startsWith(moduleName, "../"))
	{
		js_getproperty(J, 0, "dirpath");
		if (!js_isstring(J, -1))
			js_typeerror(J, "invalid dirpath property");
		const char *dirpath = js_tostring(J, -1); js_pop(J, 1);
		const char *resolvedPath = u_path_resolve(dirpath, moduleName);
		if (__require_from_cache(J, resolvedPath)) 
			return;
		js_newarray(J);
		js_pushstring(J, resolvedPath);	
		js_setindex(J, -2, 0);
	}
	else if (IS_ASBOLUTE_PATH(moduleName)) {
		if (__require_from_cache(J, moduleName)) 
			return;
		js_newarray(J);
		js_pushstring(J, moduleName);	
		js_setindex(J, -2, 0);
	}
	// concatenate module name with search paths
	else 
	{
		if (__require_from_cache(J, moduleName))
			return;
		js_copy(J, 1);
		js_pushstring(J, ".js");
		js_concat(J);
		const char *moduleNameExt = js_tostring(J, -1);

		js_newarray(J);
		js_getproperty(J, 0, "paths");
		if (!js_isarray(J, -1))
			js_typeerror(J, "search paths property is not defined");
		int pathsLen = js_getlength(J, -1);
		for (int i = 0; i < pathsLen; i++)
		{
			js_getindex(J, -1, i);
			if (!js_isstring(J, -1))
				js_typeerror(J, "expected string in paths array");
			if (js_getlength(J, -1) == 0)
				js_typeerror(J, "empty path is not allowed in paths array");
			const char *path = js_tostring(J, -1);
			const char *finalPath = u_path_join(path, moduleNameExt);
			js_pushstring(J, finalPath);
			js_setindex(J, -4, i);
			js_pop(J, 1);
		}
		js_pop(J, 1);
		js_rot2pop1(J);
	}
	int searchPaths = js_getlength(J, -1);
	for (int i = 0; i < searchPaths; i++)
	{
		js_getindex(J, -1, i);
		const char *path = js_tostring(J, -1); js_pop(J, 1);
		if (__require_new_module(J, path)) 
		{
			js_rot2pop1(J); // remove search paths array
			js_getproperty(J, 0, "cache"); // store result in cache
			js_copy(J, -2); // copy result
			// store under absolute path
			if (u_str_startsWith(moduleName, "./") || u_str_startsWith(moduleName, "../"))
				js_setproperty(J, -2, path);
			// store under module name
			else
				js_setproperty(J, -2, moduleName);
			js_getproperty(J, -2, "exports");
			js_rot3pop2(J);
			return;
		}
	}
	js_error(J, "could not find module: %s", moduleName);
}

static void jsB_Module_prototype_resolve(js_State *J)
{
	if (js_isundefined(J, 0) || !js_isobject(J, 0))
		js_typeerror(J, "invalid receiver");
	if (!js_isstring(J, 1))
		js_typeerror(J, "invalid path argument");
	if (js_getlength(J, 1) == 0)
		js_rangeerror(J, "empty path argument");
	js_getproperty(J, 0, "dirpath");
	const char *wd = js_tostring(J, -1);
	js_pop(J, 1);
	const char *path = js_tostring(J, 1);
	js_pushstring(J, u_path_resolve(wd, path));
}

void jsB_Module(js_State *J) 
{
	js_getregistry(J, "jsB_Module");
	/* check cached constructor */
	if (js_isdefined(J, -1)) {
		return;
	}
	js_pop(J, 1);

	js_newobject(J); // Module Prototype
	js_newcfunction(J, jsB_Module_prototype_compile, "Module.prototype.compile", 1);
	js_defproperty(J, -2, "compile", JS_DONTENUM | JS_DONTCONF | JS_READONLY);
	js_newcfunction(J, jsB_Module_prototype_require, "Module.prototype.require", 1);
	js_defproperty(J, -2, "require", JS_DONTENUM | JS_DONTCONF | JS_READONLY);
	js_newcfunction(J, jsB_Module_prototype_resolve, "Module.prototype.resolve", 1);
	js_defproperty(J, -2, "resolve", JS_DONTENUM | JS_DONTCONF | JS_READONLY);
	js_freeze(J);
	js_newcconstructor(J, jsB_call_Module, jsB_new_Module, "Module", 2);

	js_copy(J, -1);
	js_setregistry(J, "jsB_Module");
}

void jsB_Module_define(js_State *J, void *data)
{
	const char *moduleName = (const char*)data;
	if (js_isundefined(J, 1))
		js_typeerror(J, "invalid first argument");
	jsB_Module(J);
		js_pushstring(J, moduleName);
		js_newobject(J);
			js_copy(J, 1);
			js_setproperty(J, -2, "exports");
	js_construct(J, 2);
}
