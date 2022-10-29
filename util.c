#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static char* mstrncpy(const char* src, int n) {
  char* a = malloc(n + 1);
  strncpy(a, src, n);
  a[n] = '\0';
  return a;
}

const char* read_text(const char* path) {
  char* s = 0;
  long length;
  FILE* f = fopen(path, "r");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    s = malloc(length+1);
    if (s) {
      fread(s, 1, length, f);
      s[length] = '\0';
    }
    fclose(f);
  }

  return s;
}
