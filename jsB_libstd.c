#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>
#include "jsB_libstd.h"
#include "jsB_arraybuffer.h"

#define U_NAME_FILE "StdFile"

static void timefromms(struct timeval *tv, uint64_t v)
{
	tv->tv_sec = v / 1000;
	tv->tv_usec = (v % 1000) * 1000;
}

static void jsB_LibStd_fclose(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	int result = fclose(fd);
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_fread(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_typeerror(J, "ArrayBuffer instance expected");
	}
	int size = js_checkinteger(J, 2, 0);
	int count = js_checkinteger(J, 3, 0);
	if (!js_isuserdata(J, 4, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 4, U_NAME_FILE);
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
	if ((size * count) > backStore->length) {
		js_rangeerror(J, "Buffer is too small for the read");
	}
	int rsize = fread(backStore->data, size, count, fd);
	if (ferror(fd) != 0) {
		js_rangeerror(J, "Error occured while reading %s", strerror(errno));
	}
	js_pushnumber(J, (double)rsize);
}

static void jsB_LibStd_fwrite(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_typeerror(J, "ArrayBuffer instance expected");
	}
	int size = js_checkinteger(J, 2, 0);
	int count = js_checkinteger(J, 3, 0);
	if (!js_isuserdata(J, 4, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 4, U_NAME_FILE);
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
	if ((size * count) > backStore->length) {
		js_rangeerror(J, "Data size is smaller than write size");
	}
	int wsize = fwrite(backStore->data, size, count, fd);
	if (ferror(fd) != 0) {
		js_rangeerror(J, "Error occured while writing data %s", strerror(errno));
	}
	js_pushnumber(J, (double)wsize);
}

static void jsB_LibStd_fseek(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = js_touserdata(J, 1, U_NAME_FILE);
	long int offset = (long int)js_checknumber(J, 2, 0);
	int origin = js_checkinteger(J, 3, 0);
	if (!(origin == SEEK_SET || origin == SEEK_CUR || origin == SEEK_END)) {
		js_rangeerror(J, "Invalid origin set");
	}
	int result = fseek(fd, offset, origin);
	if (result != 0) {
		js_rangeerror(J, "Error occured while seeking stream position %s", strerror(errno));
	}
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_ftell(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = js_touserdata(J, 1, U_NAME_FILE);
	long int pos = ftell(fd);
	if (pos < 0) {
		js_rangeerror(J, "Error occured while telling stream position %s", strerror(errno));
	}
	js_pushnumber(J, (double)pos);
}

static void jsB_LibStd_fopen(js_State *J)
{
	const char *path, *mode = "rb";
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "File path must be a string");
	}
	path = js_tostring(J, 1);
	if (js_isdefined(J, 2) && js_isstring(J, 2)) {
		mode = js_tostring(J, 2);
	}
	FILE *fd = fopen(path, mode);
	if (!fd) {
		js_error(J, "Cannot open file '%s': %s", path, strerror(errno));
	}
	js_newobject(J);
	js_newuserdata(J, U_NAME_FILE, (void *)fd, NULL);
}

static void jsB_LibStd_feof(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	js_pushnumber(J, (double)feof(fd));
}

static void jsB_LibStd_ferror(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	js_pushnumber(J, (double)ferror(fd));
}

static void jsB_LibStd_fflush(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	int res = fflush(fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)res);
}

static void jsB_LibStd_fgetc(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	int c = fgetc(fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)c);
}

static void jsB_LibStd_fgets(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_typeerror(J, "ArrayBuffer instance expected");
	}
	int length = (long int)js_checknumber(J, 2, 0);
	if (!js_isuserdata(J, 3, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
	if (length > backStore->length) {
		js_typeerror(J, "Specified length exceeds buffer byte size");
	}
	FILE *fd = (FILE*)js_touserdata(J, 3, U_NAME_FILE);
	char *result = fgets(backStore->data, length, fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushboolean(J, (double)(result != NULL));
}

static void jsB_LibStd_fputc(js_State *J)
{
	if (!js_isuserdata(J, 2, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	int c = js_checkinteger(J, 1, 0);
	FILE *fd = (FILE*)js_touserdata(J, 2, U_NAME_FILE);
	int result = fputc(c, fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_fputs(js_State *J)
{
	if (!jsB_ArrayBuffer_instance(J, 1)) {
		js_typeerror(J, "ArrayBuffer instance expected");
	}
	if (!js_isuserdata(J, 2, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	js_back_store_t *backStore = jsB_ArrayBuffer_backstore(J, 1);
	FILE *fd = (FILE*)js_touserdata(J, 2, U_NAME_FILE);
	int result = fputs(backStore->data, fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_remove(js_State *J)
{
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "String is expected as path argument");
	}
	const char *path = js_tostring(J, 1);
	int result = remove(path);
	if (result < 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_rename(js_State *J)
{
	if (!js_isstring(J, 1)) {
		js_typeerror(J, "String is expected as old path argument");
	}
	if (!js_isstring(J, 2)) {
		js_typeerror(J, "String is expected as new path argument");
	}
	const char *oldPath = js_tostring(J, 1);
	const char *newPath = js_tostring(J, 2);
	int result = rename(oldPath, newPath);
	if (result < 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushnumber(J, (double)result);
}

static void jsB_LibStd_rewind(js_State *J)
{
	if (!js_isuserdata(J, 1, U_NAME_FILE)) {
		js_typeerror(J, "File instance must be passed");
	}
	FILE *fd = (FILE*)js_touserdata(J, 1, U_NAME_FILE);
	rewind(fd);
	if (ferror(fd) != 0) {
		js_error(J, "%s", strerror(errno));
	}
	js_pushundefined(J);
}

static void jsB_LibStd_tmpfile(js_State *J)
{
	FILE *fd = tmpfile();
	if (!fd) {
		js_error(J, "%s", strerror(errno));
	}
	js_newobject(J);
	js_newuserdata(J, U_NAME_FILE, (void *)fd, NULL);
}

static void jsB_LibStd_getcwd(js_State *J)
{
	static char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) == NULL) 
		js_error(J, "%s", strerror(errno));
	js_pushstring(J, cwd);
}

static void jsB_LibStd_getenv(js_State *J)
{
	const char *name = js_tostring(J, 1);
	if (!name[0]) {
		js_pushundefined(J);
		return;
	}
	const char *value = getenv(name);
	if (!value) {
		js_pushundefined(J);
	} else {
		js_pushstring(J, value);
	}
}

static void jsB_LibStd_print(js_State *J)
{
	int i, top = js_gettop(J);
	for (i = 1; i < top; ++i) {
		const char *s = js_tostring(J, i);
		if (i > 1) fputc(' ', stdout);
		fputs(s, stdout);
	}
	js_pushundefined(J);
}

static void jsB_LibStd_writeFile(js_State *J)
{
	const char *filename = js_tostring(J, 1);
	const char *data = js_tostring(J, 2);
	size_t size = js_getstrsize(J, 2); 
	FILE *fd = fopen(filename, "wb+");
	if (!fd) {
		js_error(J, "Cannot open file '%s': %s", filename, strerror(errno));
	}
	size_t wsize = fwrite(data, 1, size, fd);
	if (ferror(fd) != 0) {
		js_error(J, "Cannot write file '%s': %s", filename, strerror(errno));
	}
	fclose(fd);
	js_pushnumber(J, (double)wsize);
}

static void jsB_LibStd_readFile(js_State *J)
{
	const char *filename = js_tostring(J, 1);
	FILE *fd = fopen(filename, "rb");
	if (!fd) {
		js_error(J, "Cannot open file '%s': %s", filename, strerror(errno));
	}
	if (fseek(fd, 0, SEEK_END) != 0) {
		fclose(fd);
		js_error(J, "Cannot seek in file '%s': %s", filename, strerror(errno));
	}
	size_t size = ftell(fd);
	if (size < 0) {
		fclose(fd);
		js_error(J, "Cannot tell in file '%s': %s", filename, strerror(errno));
	}
	if (fseek(fd, 0, SEEK_SET) != 0) {
		fclose(fd);
		js_error(J, "Cannot seek in file '%s': %s", filename, strerror(errno));
	}
	void *dst = js_malloc(J, size);
	size_t rsize = fread(dst, 1, size, fd);
	if (ferror(fd) != 0) {
		free(dst);
		fclose(fd);
		js_error(J, "Cannot read file '%s': %s", filename, strerror(errno));
	}
	js_pushlstring(J, dst, size);
	free(dst);
	fclose(fd);
}

static void jsB_LibStd_exit(js_State *J)
{
	exit(js_checkinteger(J, 1, 0));
}

static void jsB_LibStd_mkdir(js_State *J)
{
	const char *path = js_tostring(J, 1);
	int mode = js_checkinteger(J, 2, 0777);
	if (!path[0]) {
		js_error(J, "mkdir: expected valid path");
	}
	int status = mkdir(path, mode);
	if (status) {
		js_error(J, "Cannot create directory '%s': %s", path, strerror(errno));
	}
	js_pushnumber(J, (double)status);
}

static void jsB_LibStd_readdir(js_State *J)
{
	const char *path = js_tostring(J, 1);
	DIR *dd = opendir(path);
	if (!dd) {
		js_error(J, "Cannot read directory '%s': %s", path, strerror(errno));
	}
	int len = 0;
	struct dirent *dent;
	js_newarray(J);
	while (1) {
		errno = 0;
		dent = readdir(dd);
		if (!dent) {
			if (errno != 0) {
				closedir(dd);
				js_error(J, "Error reading directory entity: %s", strerror(errno));
			}
			break;
		}
		js_pushstring(J, dent->d_name);
		js_setindex(J, -2, len++);
	}
	closedir(dd);
}

static void jsB_LibStd_stat(js_State *J)
{
	const char *path = js_tostring(J, 1);
	struct stat st;
	int status = stat(path, &st);
	if (status) {
		js_error(J, "Cannot stat file '%s': %s", path, strerror(errno));
	}
	js_newobject(J);
	js_pushboolean(J, S_ISREG(st.st_mode));
	js_setproperty(J, -2, "isFile");
	js_pushboolean(J, S_ISDIR(st.st_mode));
	js_setproperty(J, -2, "isDirectory");
	js_pushboolean(J, S_ISLNK(st.st_mode));
	js_setproperty(J, -2, "isLink");
	js_pushnumber(J, (double)st.st_size);
	js_setproperty(J, -2, "size");
	js_pushnumber(J, (double)st.st_atime);
	js_setproperty(J, -2, "atime");
	js_pushnumber(J, (double)st.st_mtime);
	js_setproperty(J, -2, "mtime");
	js_pushnumber(J, (double)st.st_ctime);
	js_setproperty(J, -2, "ctime");
}

static void jsB_LibStd_exists(js_State *J)
{
	const char *path = js_tostring(J, 1);
	struct stat st;
	int status = stat(path, &st);
	if (status) {
		js_pushboolean(J, 0);
	} else if (js_toboolean(J, 2)) {
		js_pushboolean(J, S_ISDIR(st.st_mode));
	} else {
		js_pushboolean(J, 1);
	}
}

/* credits: https://stackoverflow.com/a/42596507 */
int __LibStd_rmtree(const char *path)
{
    struct stat stat_path, stat_entry;
    stat(path, &stat_path);
    if (S_ISDIR(stat_path.st_mode) == 0) {
        return 1;
    }
    DIR *dir;
    if ((dir = opendir(path)) == NULL) {
        return 1;
    }
    char *full_path;
    struct dirent *entry;
    size_t path_len = strlen(path);
    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        // stat for the entry
        stat(full_path, &stat_entry);
        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            if (__LibStd_rmtree(full_path) != 0) {
            	closedir(dir);
            	return 1;
            }
            continue;
        }
        // remove a file object
        if (unlink(full_path) != 0) {
        	closedir(dir);
            return 1;
        }
    }
    if (rmdir(path) != 0) {
    	closedir(dir);
        return 1;
    }
    closedir(dir);
    return 0;
}

void jsB_LibStd_rmtree(js_State *J) 
{
    const char *path = js_tostring(J, 1);
    if (__LibStd_rmtree(path)) {
    	if (errno != 0) {
    		js_error(J, "Cannot remove directory tree '%s': %s", path, strerror(errno));
    	} else {
    		js_error(J, "Cannot remove directory tree '%s'", path);
    	}
    }
    js_pushundefined(J);
}

void jsB_LibStd(js_State *J)
{
	js_newobject(J);
	js_newcfunction(J, jsB_LibStd_fopen, "fopen", 2);
	js_setproperty(J, -2, "fopen");
	js_newcfunction(J, jsB_LibStd_fclose, "fclose", 1);
	js_setproperty(J, -2, "fclose");
	js_newcfunction(J, jsB_LibStd_fread, "fread", 4);
	js_setproperty(J, -2, "fread");
	js_newcfunction(J, jsB_LibStd_fwrite, "fwrite", 4);
	js_setproperty(J, -2, "fwrite");
	js_newcfunction(J, jsB_LibStd_fseek, "fseek", 3);
	js_setproperty(J, -2, "fseek");
	js_newcfunction(J, jsB_LibStd_ftell, "ftell", 1);
	js_setproperty(J, -2, "ftell");
	js_newcfunction(J, jsB_LibStd_feof, "feof", 1);
	js_setproperty(J, -2, "feof");
	js_newcfunction(J, jsB_LibStd_ferror, "ferror", 1);
	js_setproperty(J, -2, "ferror");
	js_newcfunction(J, jsB_LibStd_fflush, "fflush", 1);
	js_setproperty(J, -2, "fflush");
	js_newcfunction(J, jsB_LibStd_fgetc, "fgetc", 1);
	js_setproperty(J, -2, "fgetc");
	js_newcfunction(J, jsB_LibStd_fgets, "fgets", 3);
	js_setproperty(J, -2, "fgets");
	js_newcfunction(J, jsB_LibStd_fputc, "fputc", 2);
	js_setproperty(J, -2, "fputc");
	js_newcfunction(J, jsB_LibStd_fputs, "fputs", 2);
	js_setproperty(J, -2, "fputs");
	js_newcfunction(J, jsB_LibStd_remove, "remove", 1);
	js_setproperty(J, -2, "remove");
	js_newcfunction(J, jsB_LibStd_rename, "rename", 2);
	js_setproperty(J, -2, "rename");
	js_newcfunction(J, jsB_LibStd_rewind, "rewind", 1);
	js_setproperty(J, -2, "rewind");
	js_newcfunction(J, jsB_LibStd_tmpfile, "tmpfile", 1);
	js_setproperty(J, -2, "tmpfile");
	js_newcfunction(J, jsB_LibStd_getcwd, "getcwd", 0);
	js_setproperty(J, -2, "getcwd");
	js_newcfunction(J, jsB_LibStd_getenv, "getenv", 1);
	js_setproperty(J, -2, "getenv");
	js_newcfunction(J, jsB_LibStd_print, "print", 0);
	js_setproperty(J, -2, "print");
	js_newcfunction(J, jsB_LibStd_readFile, "readFile", 1);
	js_setproperty(J, -2, "readFile");
	js_newcfunction(J, jsB_LibStd_writeFile, "writeFile", 2);
	js_setproperty(J, -2, "writeFile");
	js_newcfunction(J, jsB_LibStd_exit, "exit", 1);
	js_setproperty(J, -2, "exit");
	js_newcfunction(J, jsB_LibStd_mkdir, "mkdir", 2);
	js_setproperty(J, -2, "mkdir");
	js_newcfunction(J, jsB_LibStd_readdir, "readdir", 1);
	js_setproperty(J, -2, "readdir");
	js_newcfunction(J, jsB_LibStd_stat, "stat", 1);
	js_setproperty(J, -2, "stat");
	js_newcfunction(J, jsB_LibStd_exists, "exists", 1);
	js_setproperty(J, -2, "exists");
	js_newcfunction(J, jsB_LibStd_rmtree, "rmtree", 1);
	js_setproperty(J, -2, "rmtree");
	js_pushnumber(J, SEEK_SET);
	js_setproperty(J, -2, "SEEK_SET");
	js_pushnumber(J, SEEK_CUR);
	js_setproperty(J, -2, "SEEK_CUR");
	js_pushnumber(J, SEEK_END);
	js_setproperty(J, -2, "SEEK_END");
	js_pushnumber(J, EOF);
	js_setproperty(J, -2, "EOF");
	/* std output */
	js_newobject(J);
	js_newuserdata(J, U_NAME_FILE, (void*)stdout, NULL);
	js_setproperty(J, -2, "stdout");
	/* std input */
	js_newobject(J);
	js_newuserdata(J, U_NAME_FILE, (void*)stdin, NULL);
	js_setproperty(J, -2, "stdin");
	js_freeze(J);
}
