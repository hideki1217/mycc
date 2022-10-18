#pragma once

#include <stdbool.h>

#include "define.h"

typedef unsigned int uint;
typedef unsigned long ulong;

// util.c

typedef struct Vec {
  void** buf;
  uint buf_l;
  uint len;
} Vec;
Vec* Vec_new();
void Vec_push(Vec* self, void* item);
void* Vec_pop(Vec* self);
void* Vec_get(const Vec* self, uint idx);
bool Vec_empty(const Vec* self);
void Vec_free(Vec* self);

#define MAP_KEY_MAX 64
typedef struct Map {
  void* root;
  uint len;
} Map;
Map* Map_new();
bool Map_push(Map* self, const char* key, int n, void* item);
void *Map_pushf(Map* self, const char* key, int n, void* item);
void* Map_pop(Map* self, const char* key, int n);
void* Map_get(Map* self, const char* key, int n);
bool Map_empty(Map* self);
void Map_free(Map* self);

// tokenize.c
typedef struct Token {
  IDs id;
  const char* pos;
  struct Token* next;  // NULLなら終端

  // Ident
  // Str
  const char* corrected; // 通常のstring.hが適用可能

  // Const Integer
  long const_int;

  // Const Float
  // double const_float;
} Token;

extern Token* tokenize(const char* content);
