#ifndef TEST_SHARED_H
#define TEST_SHARED_H

#ifdef __cplusplus
  extern "C" {
#endif

int get_file_contents(const char *path, int *size, char **buffer);

#ifdef __cplusplus
}
#endif

#endif // TEST_SHARED_H