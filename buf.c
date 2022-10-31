#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static void Buf_update(Buf* self) {
  if (self->buf_len <= self->len + 1) {
    if (self->buf_len == 0) {
      self->buf_len = 8;
      self->p = malloc(self->buf_len);
      return;
    }
    char* p = malloc(self->buf_len * 2);
    memcpy(p, self->p, self->buf_len);
    free(self->p);
    self->p = p;
    self->buf_len *= 2;
  }
}
Buf* Buf_new() { Buf_withsize(0); }
Buf* Buf_withsize(int size) {
  Buf* self = malloc(sizeof(Buf));
  self->buf_len = size;
  self->p = (size > 0) ? malloc(self->buf_len) : NULL;
  self->len = 0;
  return self;
}
void Buf_clear(Buf* self) { self->len = 0; }
void Buf_writec(Buf* self, char c) {
  Buf_update(self);
  self->p[self->len++] = c;
}
void Buf_writes(Buf* self, const char* s) {
  for (; *s != '\0'; s++) Buf_writec(self, *s);
}
void Buf_eof(Buf* self) {
  if ((self->p)[self->len - 1] != '\0') {
    (self->p)[self->len] = '\0';
  }
}
const char* Buf_ref(Buf* self) {
  Buf_eof(self);
  return self->p;
}
char* Buf_into_str(Buf* self) {
  Buf_eof(self);
  const char* res = self->p;
  free(self);
  return res;
}
