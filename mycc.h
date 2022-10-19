#pragma once

#include <stdbool.h>

#include "define.h"

typedef unsigned int uint;
typedef unsigned long ulong;

// util.c

#define Vec_define(Self, Type)                  \
  typedef struct {                              \
    Type* buf;                                  \
    uint buf_l;                                 \
    uint len;                                   \
  } Self;                                       \
  Self* Self##_new();                           \
  Type* Self##_push(Self* self);                \
  bool Self##_pop(Self* self);                  \
  Type* Self##_get(const Self* self, uint idx); \
  bool Self##_empty(const Self* self);          \
  void Self##_free(Self* self);

Vec_define(PtrV, void*);
Vec_define(IntV, int);
#undef Vec_define

typedef struct Map {
  void* root;
  uint len;
} Map;
Map* Map_new();
bool Map_push(Map* self, const char* key, int n, void* item);
void* Map_pushf(Map* self, const char* key, int n, void* item);
void* Map_pop(Map* self, const char* key, int n);
void* Map_get(Map* self, const char* key, int n);
bool Map_empty(Map* self);
void Map_free(Map* self);

typedef struct {
  void* root;
  uint len;
} Map_;
Map_* Map__new();
bool Map__push(Map* self, const char* key, int n, void* item);
void* Map__pushf(Map* self, const char* key, int n, void* item);
void* Map__pop(Map* self, const char* key, int n);
void* Map__get(Map* self, const char* key, int n);
bool Map__empty(Map* self);
void Map__free(Map* self);

// tokenize.c
typedef struct Token {
  IDs id;
  const char* pos;
  struct Token* next;  // NULLなら終端

  // Ident
  // Str
  const char* corrected;  // 通常のstring.hが適用可能

  // Const Integer
  long const_int;

  // Const Float
  // double const_float;
} Token;

extern Token* tokenize(const char* content);
