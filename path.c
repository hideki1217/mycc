#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "mycc.h"

int pathisabs(const char* path) { return *path == '/'; }

static void _pathnorm(char* dst, const char* src) {
  char* cur = dst;
  while (*src != '\0') {
    if (strncmp(src, "/..", 3) == 0 && (src[3] == '/' || src[3] == '\0')) {
      src += 3;
      while (*cur != '/' && cur > dst) cur--;
      continue;
    }
    if (strncmp(src, "/.", 2) == 0 && (src[2] == '/' || src[2] == '\0')) {
      src += 2;
      continue;
    }
    assert(*src == '/');
    *(cur++) = *(src++);
    while (*src != '/' && *src != '\0') 
      *(cur++) = *(src++);
  }
  *cur = '\0';
}

char* pathnorm(char* dst, const char* path) {
  *dst = '/';
  _pathnorm(dst, path);
  return dst;
}

char* pathname(char* dst, const char* path) {
  char tmp[MAX_PATH];
  char* start = dst;
  pathnorm(tmp, path);
  const char* cur = tmp + (strlen(tmp) - 1);
  while (*cur != '/' && cur > path) cur--;
  cur++;
  while (*cur != '\0') *dst++ = *cur++;
  *dst = '\0';
  return start;
}

char* pathparen(char* dst, const char* path) {
  char tmp[MAX_PATH];
  pathnorm(tmp, path);
  const char* end = tmp;
  char* start = dst;
  for (const char* cur = tmp; *cur != '\0'; cur++) {
    if (*cur == '/') end = cur;
  }
  for (const char* cur = tmp; cur != end;) {
    *(dst++) = *(cur++);
  }
  *dst = '\0';
  return start;
}

char* pathcat(char* base, const char* relative) {
  assert(!pathisabs(relative));
  assert(pathisabs(base));

  char tmp[MAX_PATH];
  pathnorm(tmp, base);
  strcat(tmp, "/");
  strcat(tmp, relative);
  strcpy(base, tmp);
  return base;
}
