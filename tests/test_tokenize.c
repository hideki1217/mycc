#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"

int main() {
  const char* path = "/home/hideki/Documents/repositories/mycc/tests/test_tokenize.txt";

  char* s = 0;
  long length;
  FILE* f = fopen(path, "rb");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    s = malloc(length);
    if (s) {
      fread(s, 1, length, f);
    }
    fclose(f);
  }

  if (s) {
    Token* res = tokenize(path, s);
    Token* tmp = res;
    while (tmp != NULL) {
      tmp = tmp->next;
    }
  }
  else {
    printf("file is not found");
    return 1;
  }
}