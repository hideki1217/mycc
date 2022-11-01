#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

Vec* Vec_new() { return Vec_withsize(0); }
Vec* Vec_withsize(int size) {
  Vec* x = malloc(sizeof(Vec));
  x->buf_len = size;
  x->buf = size > 0 ? malloc(sizeof(void*) * x->buf_len) : NULL;
  x->len = 0;
  return x;
}
static void Vec_update(Vec* self) {
  if (self->buf_len == self->len) {
    if (self->buf_len == 0) {
      self->buf_len = 8;
      self->buf = malloc(sizeof(void*) * self->buf_len);
      return;
    }
    void** buf = malloc(sizeof(void*) * self->buf_len * 2);
    memcpy(buf, self->buf, sizeof(void*) * self->buf_len);
    free(self->buf);
    self->buf = buf;
    self->buf_len *= 2;
  }
}
void* Vec_push(Vec* self, void* x) {
  Vec_update(self);
  self->buf[self->len++] = x;
  return x;
}
void* Vec_pop(Vec* self) {
  if (Vec_empty(self)) return NULL;
  self->len -= 1;
  void* x = self->buf[self->len];
  return x;
}
void* Vec_get(Vec* self, int idx) {
  assert(self->len > idx && idx >= 0);
  return self->buf[idx];
}
Vec* Vec_clear(Vec* self) {
  self->len = 0;
  return self;
}
bool Vec_empty(const Vec* self) { return self->len == 0; }
static void reverse(void** buf, int n) {
  for (int i = 0; i < n / 2; i++) {
    void* tmp = buf[i];
    buf[i] = buf[n - 1 - i];
    buf[n - 1 - i] = tmp;
  }
}
Vec* Vec_reverse(Vec* self) {
  reverse(self->buf, self->len);
  return self;
}
void Vec_free(Vec* self) {
  free(self->buf);
  free(self);
}