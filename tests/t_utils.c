#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "t_utils.h"

int get_file_contents(const char *path, int *size, char **buffer)
{
	FILE *f;
	int len, rlen;
	*buffer = NULL;
	*size = 0;
	f = fopen(path, "rb");
	if (!f) {
		return errno;
	}
	if (fseek(f, 0, SEEK_END) < 0) {
		fclose(f);
		return errno;
	}
	len = ftell(f);
	if (len < 0) {
		fclose(f);
		return errno;
	}
	if (fseek(f, 0, SEEK_SET) < 0) {
		fclose(f);
		return errno;
	}
	*buffer = malloc(len + 1);
	if (!*buffer) {
		fclose(f);
		return errno;
	}
	rlen = fread(*buffer, 1, len, f);
	if (rlen != len) {
		free(*buffer);
		fclose(f);
		return errno;
	}
	(*buffer)[rlen] = 0;
	fclose(f);
	*size = len;
	return 0;
}
