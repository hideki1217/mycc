#pragma once

#include <stdbool.h>

#include "define.h"

typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct Token Token;

// util.c
const char* read_text(const char* path);

#define Vec_define(Self, Type)                  \
  typedef struct {                              \
    Type* buf;                                  \
    uint buf_l;                                 \
    uint len;                                   \
  } Self;                                       \
  Self* Self##_new();                           \
  Self* Self##_withsize(int size);              \
  Type* Self##_push(Self* self);                \
  bool Self##_pop(Self* self);                  \
  Type* Self##_get(const Self* self, uint idx); \
  bool Self##_empty(const Self* self);          \
  void Self##_clear(Self* self);                \
  void Self##_free(Self* self);

Vec_define(Vec, void*);
Vec_define(IntV, int);
Vec_define(StrV, const char*);
#undef Vec_define

typedef struct {
  Token** buf;
  long buf_len;
  long len;
} TokenS;
TokenS* TokenS_new();
TokenS* TokenS_withsize(int size);
Token* TokenS_push(TokenS* self, Token* tk);
Token* TokenS_pop(TokenS* self);
Token* TokenS_get(TokenS* self, int idx);
TokenS* TokenS_clear(TokenS* self);
bool TokenS_empty(const TokenS* self);
void TokenS_free(TokenS* self);

typedef struct {
  const char* x;
  int lhs;
  int rhs;
} DictEntry;
typedef struct Dict {
  DictEntry* buf;
  int buf_len;
  int len;
} Dict;
Dict* Dict_new();
const char* Dict_push(Dict* self, const char* s);
const char* Dict_push_copy(Dict* self, const char* s);
bool Dict_empty(const Dict* self);
bool Dict_contain(const Dict* self, const char* s);
void Dict_free(Dict* self);

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
  Dict* dict;
  StrV* include_path;
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

extern TokenS* tokenize(Context* context);

// pp.c
#define MAX_INCLUDE_DEPTH 6
TokenS* preprocess(Context* context, TokenS* input);
