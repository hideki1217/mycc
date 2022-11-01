#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"

Pos* Pos_new(const char* file, int x) {
  Pos* self = malloc(sizeof(Pos));
  self->file = file;
  self->x = x;
  return self;
}

#define MAX_LINE 256
void get_line(Pos* p, int* x, int* y, char* line) {
  FILE* f = fopen(p->file, "r");
  assert(f != NULL);

  int c, x_ = 0, y_ = 0;
  char* cur = line;
  for (int i = 0; i < p->x; i++) {
    c = fgetc(f);
    assert(c != EOF);

    if (c == '\n') {
      x_ = 0;
      y_++;
      cur = line;
    } else {
      x_++;
      *cur++ = c;
    }
  }
  while ((c = fgetc(f)) != EOF) {
    if (c == '\n') break;
    *cur++ = c;
  }
  *cur = '\0';

  fclose(f);

  assert(x_ < MAX_LINE);

  *x = x_;
  *y = y_;
}

static void print_pos_(const char* line, int x, int y) {
  int n;
  printf("%5d | %s\n", y + 1, line);
  printf("%5s | ", "");
  for (int i = 0; i < x; i++) printf(" ");
  printf("^~~~\n");
}

void errorf(Pos* p, const char* fmt, ...) {
  assert(p != NULL && fmt != NULL);

  int x, y;
  char line[MAX_LINE];
  get_line(p, &x, &y, line);

  printf("%s:%d:%d: error: ", p->file, y + 1, x + 1);
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");

  print_pos_(line, x, y);
}

void warnf(Pos* p, const char* fmt, ...) {
  assert(p != NULL && fmt != NULL);

  int x, y;
  char line[MAX_LINE];
  get_line(p, &x, &y, line);

  printf("%s:%d:%d: warning: ", p->file, y + 1, x + 1);
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");

  print_pos_(line, x, y);
}
