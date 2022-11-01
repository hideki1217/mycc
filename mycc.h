#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "define.h"

#define TODO() abort()

typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct Token Token;

// util.c
const char* read_text(const char* path);

typedef struct {
  void** buf;
  long buf_len;
  long len;
} Vec;
Vec* Vec_new();
Vec* Vec_withsize(int size);
void* Vec_push(Vec* self, void* x);
void* Vec_pop(Vec* self);
void* Vec_get(Vec* self, int idx);
/*return self*/
Vec* Vec_clear(Vec* self);
bool Vec_empty(const Vec* self);
/*return self*/
Vec* Vec_reverse(Vec* self);
void Vec_free(Vec* self);

typedef struct DictNode* Dict;
#define DICT_EMPTY NULL
#define Dict_new() DICT_EMPTY
const char* Dict_push(Dict* self, const char* s);
const char* Dict_push_copy(Dict* self, const char* s);
#define Dict_empty(self) ((self) == DICT_EMPTY)
const char* Dict_contain(Dict self, const char* s);
void Dict_free(Dict t);

typedef struct mapNode* Map;
#define MAP_EMPTY NULL
#define Map_new() MAP_EMPTY
void Map_free(Map t);
void* Map_contain(Map t, long key);
int Map_push(Map* t, long key, void* item);
void* Map_pushf(Map* t, long key, void* item);
void Map_print_keys(Map t);
void* Map_delete(Map* t, long key);

typedef struct Buf {
  char* p;
  int buf_len;
  int len;
} Buf;
Buf* Buf_new();
Buf* Buf_withsize(int n);
void Buf_clear(Buf* self);
void Buf_writec(Buf* self, char c);
void Buf_writes(Buf* self, const char* s);
void Buf_eof(Buf* self);
const char* Buf_ref(Buf* self);
char* Buf_into_str(Buf* self);

typedef struct setNode* Set;
#define SET_EMPTY NULL

#define Set_new() SET_EMPTY
/*return *self*/
Set Set_push(Set* self, long item);
Set Set_cpy(Set self);
bool Set_contain(Set self, long item);
Set Set_union(Set self, Set rhs);
Set Set_union_asgn(Set* self, Set rhs);
Set Set_cross(Set self, Set rhs);
void Set_free(Set set);

// path.c
#define MAX_PATH 512
int pathisabs(const char* path);
char* pathnorm(char* dst, const char* path);
char* pathname(char* dst, const char* path);
char* pathparen(char* dst, const char* path);
char* pathcat(char* base, const char* relative);

typedef struct {
  const char* name;
  const char* content;
  Dict dict;
  Vec* include_path;
} Context;

// error.c

const char* position_info(const char* content, const char* pos, int* x, int* y);
void print_error(const char* name, const char* content, const char* pos,
                 char* fmt, ...);
void print_warning(const char* name, const char* content, const char* pos,
                   char* fmt, ...);

// tokenize.c
#define MAX_IDENT 128
struct Token {
  IDs id;
  const char* pos;
  Set hideset;  // 所有権あり

  // Ident
  // Str
  // Include path
  const char* corrected;  // 通常のstring.hが適用可能 所有権を持ってない

  // Const Integer
  long const_int;

  // Const Float
  // double const_float;
};
#define tk_eof(tk) ((tk)->id == ID_EOF)
Token* Token_new(IDs id, const char* pos);
void Token_free(Token* tk);
Token* Token_cpy(Token* tk);
Buf* tk2s(Token* tk, Buf* buf);

extern Vec* tokenize(Context* context);

// pp.c
#define MAX_INCLUDE_DEPTH 6
Vec* preprocess(Context* context, Vec* input);
