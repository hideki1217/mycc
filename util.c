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

  return s;
}

#define Vec_define(Self, Type)                                   \
  Self* Self##_new() {                                           \
    Self* self = malloc(sizeof(Self));                           \
    self->buf_l = 8;                                             \
    self->buf = malloc(sizeof(Type) * self->buf_l);              \
    self->len = 0;                                               \
    return self;                                                 \
  }                                                              \
  Self* Self##_withsize(int size) {                              \
    Self* self = malloc(sizeof(Self));                           \
    self->buf_l = size;                                          \
    self->buf = malloc(sizeof(Type) * self->buf_l);              \
    self->len = 0;                                               \
    return self;                                                 \
  }                                                              \
  Type* Self##_push(Self* self) {                                \
    if (self->buf_l == self->len) {                              \
      Type* dst = malloc(sizeof(Type) * self->buf_l * 2);        \
      memcpy(dst, self->buf, sizeof(Type) * self->buf_l);        \
      Type* tmp = self->buf;                                     \
      self->buf = dst;                                           \
      self->buf_l *= 2;                                          \
      free(tmp);                                                 \
    }                                                            \
    void* tmp = &self->buf[self->len];                           \
    self->len += 1;                                              \
    return tmp;                                                  \
  }                                                              \
  bool Self##_pop(Self* self) {                                  \
    if (!Self##_empty(self)) {                                   \
      self->len--;                                               \
      return true;                                               \
    }                                                            \
    return false;                                                \
  }                                                              \
  Type* Self##_get(const Self* self, uint idx) {                 \
    assert(self->len > idx);                                     \
    return &(self->buf)[idx];                                    \
  }                                                              \
  bool Self##_empty(const Self* self) { return self->len == 0; } \
  void Self##_clear(Self* self) { self->len = 0; }               \
  void Self##_free(Self* self) {                                 \
    free(self->buf);                                             \
    free(self);                                                  \
  }
Vec_define(Vec, void*);
Vec_define(IntV, int);
Vec_define(StrV, const char*);
#undef Vec_define

TokenS* TokenS_new() { return TokenS_withsize(8); }
TokenS* TokenS_withsize(int size) {
  TokenS* x = malloc(sizeof(TokenS));
  x->buf_len = size;
  x->buf = malloc(sizeof(Token*) * x->buf_len);
  x->len = 0;
  return x;
}
static void TokenS_update(TokenS* self) {
  if (self->buf_len == self->len) {
    Token** buf = malloc(sizeof(Token*) * self->buf_len * 2);
    memcpy(buf, self->buf, sizeof(Token*) * self->buf_len);
    free(self->buf);
    self->buf = buf;
    self->buf_len *= 2;
  }
}
Token* TokenS_push(TokenS* self, Token* tk) {
  TokenS_update(self);
  self->buf[self->len++] = tk;
  return tk;
}
Token* TokenS_pop(TokenS* self) {
  if (TokenS_empty(self)) return NULL;
  self->len -= 1;
  Token* tk = self->buf[self->len];
  return tk;
}
Token* TokenS_get(TokenS* self, int idx) {
  assert(self->len > idx && idx >= 0);
  return self->buf[idx];
}
TokenS* TokenS_clear(TokenS* self) {
  self->len = 0;
  return self;
}
bool TokenS_empty(const TokenS* self) { return self->len == 0; }
void TokenS_free(TokenS* self) {
  free(self->buf);
  free(self);
}

Dict* Dict_new() {
  Dict* self = malloc(sizeof(Dict));

  self->buf_len = 8;
  self->buf = malloc(sizeof(DictEntry) * self->buf_len);
  self->len = 0;

  return self;
}
static void Dict_update(Dict* self) {
  if (self->buf_len == self->len) {
    DictEntry* buf = malloc(sizeof(DictEntry) * self->buf_len * 2);
    memcpy(buf, self->buf, sizeof(DictEntry) * self->buf_len);

    free(self->buf);
    self->buf = buf;
    self->buf_len *= 2;
  }
}
static const char* _Dict_push(Dict* self, DictEntry* now, const char* key) {
  int res = strcmp(now->x, key);
  if (res > 0) {
    if (now->lhs == -1) {
      DictEntry* pos = &(self->buf)[self->len];
      pos->x = key;
      pos->lhs = pos->rhs = -1;

      now->lhs = self->len++;
      return key;
    }
    return _Dict_push(self, &(self->buf)[now->lhs], key);
  }
  if (res < 0) {
    if (now->rhs == -1) {
      DictEntry* pos = &(self->buf)[self->len];
      pos->x = key;
      pos->lhs = pos->rhs = -1;

      now->rhs = self->len++;
      return key;
    }
    return _Dict_push(self, &(self->buf)[now->rhs], key);
  }
  // res == 0
  return now->x;
}
const char* Dict_push(Dict* self, const char* s) {
  Dict_update(self);
  if (Dict_empty(self)) {
    DictEntry* x = self->buf;
    x->x = s;
    x->lhs = -1;
    x->rhs = -1;
    self->len++;
    return s;
  }
  return _Dict_push(self, self->buf, s);
}
static const char* _Dict_push_copy(Dict* self, DictEntry* now,
                                   const char* key) {
  int res = strcmp(now->x, key);
  if (res > 0) {
    if (now->lhs == -1) {
      DictEntry* pos = &(self->buf)[self->len];
      pos->x = mstrncpy(key, strlen(key));
      pos->lhs = pos->rhs = -1;

      now->lhs = self->len++;
      return pos->x;
    }
    return _Dict_push_copy(self, &(self->buf)[now->lhs], key);
  }
  if (res < 0) {
    if (now->rhs == -1) {
      DictEntry* pos = &(self->buf)[self->len];
      pos->x = mstrncpy(key, strlen(key));
      pos->lhs = pos->rhs = -1;

      now->rhs = self->len++;
      return pos->x;
    }
    return _Dict_push_copy(self, &(self->buf)[now->rhs], key);
  }
  // res == 0
  return now->x;
}
const char* Dict_push_copy(Dict* self, const char* s) {
  Dict_update(self);
  if (Dict_empty(self)) {
    DictEntry* x = self->buf;
    x->x = mstrncpy(s, strlen(s));
    x->lhs = -1;
    x->rhs = -1;

    self->len++;
    return x->x;
  }
  return _Dict_push_copy(self, self->buf, s);
}
bool Dict_empty(const Dict* self) { return self->len == 0; }
bool _Dict_contain(const Dict* self, const DictEntry* now, const char* s) {
  int res = strcmp(now->x, s);
  if (res < 0)
    return (now->lhs == -1) ? false
                            : _Dict_contain(self, self->buf + now->lhs, s);
  if (res > 0)
    return (now->rhs == -1) ? false
                            : _Dict_contain(self, self->buf + now->rhs, s);
  return true;
}
bool Dict_contain(const Dict* self, const char* s) {
  if (Dict_empty(self)) return false;
  return _Dict_contain(self, self->buf, s);
}
void Dict_free(Dict* self) {
  for (int i = 0; i < self->len; i++) {
    free((void*)self->buf[i].x);
  }
  free(self->buf);
  free(self);
}
