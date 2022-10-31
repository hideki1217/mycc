#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
// #include <math.h>

#include "define.h"
#include "mycc.h"

static int is_space(char c) { return c == ' ' || c == '\n' || c == '\t'; }
static int is_digit(char c) { return '0' <= c && c <= '9'; }
static int is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static const char* next(const char* s) {
  while (strncmp(s, "\\\n", sizeof("\\\n") - 1) == 0) {
    s += sizeof("\\\n") - 1;
  }
  s++;
  while (strncmp(s, "\\\n", sizeof("\\\n") - 1) == 0) {
    s += sizeof("\\\n") - 1;
  }
  return s;
}
static const char* next_n(const char* s, int n) {
  assert(n > 0);
  for (int i = 0; i < n; i++) {
    if (*s == '\0') return NULL;
    s = next(s);
  }
  return s;
}
static const char* equal_n(const char* s, const char* x, int n) {
  for (int i = 0; i < n; i++) {
    if (*s != *x) return NULL;
    s = next(s);
    x = next(x);
  }
  return s;
}
static void copy_n(char* dst, const char* src, int n) {
  const char* tmp = src;
  for (int i = 0; i < n; i++) {
    dst[i] = *tmp;
    tmp = next(tmp);
  }
}
static const char* find_char(const char* s, char c) {
  while (*s != '\0') {
    if (*s == c) return s;
    s = next(s);
  }
  return NULL;
}
static const char* find_str(const char* s, const char* x, int x_len) {
  while (*s != '\0') {
    if (equal_n(s, x, x_len)) return s;
    s = next(s);
  }
  return NULL;
}
static bool match(const char* s, const char** ls, const char* word,
                  int word_n) {
  const char* tmp;
  if (tmp = equal_n(s, word, word_n)) {
    *ls = tmp;
    return true;
  }
  return false;
}
static bool match_strict(const char* s, const char** ls, const char* word,
                         int word_n) {
  const char* tmp;
  if ((tmp = equal_n(s, word, word_n)) && is_space(*tmp)) {
    *ls = tmp;
    return true;
  }
  return false;
}

static int char2int(char c, int base) {
  assert(0 < base);
  assert(base <= 36);
  if (base <= 10) {
    if ('0' <= c && c < '0' + base) return c - '0';
  } else {
    if ('0' <= c && c < '0' + 10) return c - '0';
    if ('a' <= c && c < 'a' + (base - 10)) return c - 'a' + 10;
  }
  return -1;
}
static long strntol(const char* s, const char** ls, int base, int n) {
  assert(base >= 2);
  long x = 0;
  int val;
  for (int i = 0; i < n; i++) {
    if ((val = char2int(*s, base)) >= 0) {
      x *= base;
      x += val;
      s = next(s);
    } else
      break;
  }
  *ls = s;
  return x;
}
// static
// double strntod(const char* s, const char** ls, int n) {
//   assert(n >= 0);
//   int x_n, y_n;
//   long x, y;
//   const char *tmp;
//   x = strntol(s, &tmp, 10, n);
//   x_n = tmp - s;
//   if(*tmp != '.') return x;
//   s = next(tmp);
//   y = strntol(s, &tmp, 10, n - x_n);
//   y_n = tmp - s;

//   return (double)x + (double)y * pow(10, -y_n);
// }
static char read_char(const char* s, const char** ls) {
  if (*s == '\\') {
    if (s[1] == 'a') {
      *ls = next_n(s, 2);
      return '\a';
    }
    if (s[1] == 'b') {
      *ls = next_n(s, 2);
      return '\b';
    }
    if (s[1] == 'f') {
      *ls = next_n(s, 2);
      return '\f';
    }
    if (s[1] == 'n') {
      *ls = next_n(s, 2);
      return '\n';
    }
    if (s[1] == 'r') {
      *ls = next_n(s, 2);
      return '\r';
    }
    if (s[1] == 't') {
      *ls = next_n(s, 2);
      return '\t';
    }
    if (s[1] == 'v') {
      *ls = next_n(s, 2);
      return '\v';
    }
    if (s[1] == '\\') {
      *ls = next_n(s, 2);
      return '\\';
    }
    if (s[1] == '\?') {
      *ls = next_n(s, 2);
      return '\?';
    }
    if (s[1] == '\'') {
      *ls = next_n(s, 2);
      return '\'';
    }
    if (s[1] == '\"') {
      *ls = next_n(s, 2);
      return '\"';
    }
    if (s[1] == 'x') {
      /*16進数２桁*/
      return strntol(next_n(s, 2), ls, 16, 2);
    }
    /*8進数3桁*/
    return strntol(next(s), ls, 8, 3);
  } else {
    *ls = next(s);
    return *s;
  }
}

Token* Token_new(IDs id, const char* pos) {
  assert(pos != NULL);
  Token* res = malloc(sizeof(Token));
  res->id = id;
  res->pos = pos;
  res->corrected = NULL;
  res->hideset = SET_EMPTY;
  return res;
}
void Token_free(Token* tk) {
  Set_free(tk->hideset);
  free(tk);
}
Token* Token_cpy(Token* tk) {
  Token* res = malloc(sizeof(Token));
  memcpy(res, tk, sizeof(Token));
  res->hideset = Set_cpy(res->hideset);
  return res;
}

Vec* tokenize(Context* context) {
  Vec* tks = Vec_new();

  const char* cur = context->content;
#define ERROR(pos_, ...)                                               \
  do {                                                                 \
    print_error(context->name, context->content, (pos_), __VA_ARGS__); \
    abort();                                                           \
  } while (false)

  Token* tk;
  int macro_mode = false;
  while (*cur != '\0') {
    if (macro_mode) {
      if (*cur == '\n') {
        tk = Vec_push(tks, Token_new(ID_PP_END, cur));

        cur = next(cur);
        macro_mode = false;
        continue;
      }
    }
    if (is_space(*cur)) {
      cur = next(cur);
      continue;
    }
    if (match(cur, &cur, S_LINE_COM, sizeof(S_LINE_COM) - 1)) {
      const char* tmp = find_char(cur, '\n');
      if (tmp == NULL) break;  // find_char read to end
      cur = tmp;
      continue;
    }
    if (match(cur, &cur, S_BLCK_COML, sizeof(S_BLCK_COML) - 1)) {
      const char* tmp = find_str(cur, S_BLCK_COMR, sizeof(S_BLCK_COMR) - 1);
      if (tmp == NULL)
        ERROR(cur - sizeof(S_BLCK_COML) + 1, "Comment is not closed");
      cur = next_n(tmp, sizeof(S_BLCK_COMR) - 1);
      continue;
    }
    if (macro_mode == ID_PP_INCLUDE) {
      if (*cur == '<') {
        char path[MAX_IDENT + 1];
        const char *last_not_space = cur, *tmp = next(cur);
        for (int i = 0;; i++) {
          if (*tmp == '>') {
            path[i] = '\0';
            assert(i <= MAX_IDENT);
            break;
          }
          if (*tmp == '\n') {
            ERROR(last_not_space, "missing terminating > character");
          }
          if (!is_space(*tmp)) last_not_space = tmp;
          path[i] = *tmp;
          tmp = next(tmp);
        }

        tk = Vec_push(tks, Token_new(ID_PP_INCLUDE_PATH, cur));
        tk->corrected = Dict_push_copy(&context->dict, path);

        cur = next(tmp);
        continue;
      }
    }

#define match_symbol(name_)                                   \
  {                                                           \
    const char* tmp;                                          \
    if (match(cur, &tmp, S_##name_, sizeof(S_##name_) - 1)) { \
      tk = Vec_push(tks, Token_new(ID_##name_, cur));         \
      cur = tmp;                                              \
      continue;                                               \
    }                                                         \
  }
    match_symbol(USHFT_ASGN);
    match_symbol(DSHFT_ASGN);
    match_symbol(CCC);

    match_symbol(USHFT);
    match_symbol(DSHFT);
    match_symbol(ADD_ASGN);
    match_symbol(SUB_ASGN);
    match_symbol(DIV_ASGN);
    match_symbol(MOD_ASGN);
    match_symbol(AND_ASGN);
    match_symbol(OR_ASGN);
    match_symbol(XOR_ASGN);
    match_symbol(OR_LESS);
    match_symbol(OR_GRTR);
    match_symbol(EQL);
    match_symbol(NOT_EQL);
    match_symbol(OR_LESS);
    match_symbol(LGAND);
    match_symbol(LGOR);
    match_symbol(ARROW);

    match_symbol(ADD);
    match_symbol(SUB);
    match_symbol(MUL);
    match_symbol(DIV);
    match_symbol(MOD);
    match_symbol(LGNOT);
    match_symbol(NOT);
    match_symbol(AND);
    match_symbol(OR);
    match_symbol(XOR);
    match_symbol(LESS_THN);
    match_symbol(GRTR_THN);
    match_symbol(INSERT);
    match_symbol(L0);
    match_symbol(R0);
    match_symbol(L1);
    match_symbol(R1);
    match_symbol(L2);
    match_symbol(R2);
    match_symbol(C0);
    match_symbol(C1);
    match_symbol(C2);
    match_symbol(QUEST);
    match_symbol(C);  // WARNING: conflict Float

    match_symbol(PP_CONCAT);
#define match_reserved(name_)                                        \
  {                                                                  \
    const char* tmp;                                                 \
    if (match_strict(cur, &tmp, S_##name_, sizeof(S_##name_) - 1)) { \
      tk = Vec_push(tks, Token_new(ID_##name_, cur));                \
      cur = tmp;                                                     \
      continue;                                                      \
    }                                                                \
  }
    match_reserved(IF);
    match_reserved(ELSE);
    match_reserved(DO);
    match_reserved(GOTO);
    match_reserved(SWITCH);
    match_reserved(WHILE);
    match_reserved(FOR);
    match_reserved(CONTNE);
    match_reserved(BREAK);
    match_reserved(RETURN);
    match_reserved(CASE);
    match_reserved(DFLT);
    match_reserved(STRCT);
    match_reserved(UNION);
    match_reserved(ENUM);
    match_reserved(VOID);
    match_reserved(CHAR);
    match_reserved(SHRT);
    match_reserved(INT);
    match_reserved(LONG);
    match_reserved(FLOAT);
    match_reserved(DOBLE);
    match_reserved(TYPEDEF);
    match_reserved(STATIC);
    match_reserved(EXTERN);
    match_reserved(CONST);
    match_reserved(VLTLE);

    {  // PP_SYMBL
      const char* tmp;
      if (match(cur, &tmp, S_PP_SYMBL, sizeof(S_PP_SYMBL) - 1)) {
        tk = Vec_push(tks, Token_new(ID_PP_SYMBL, cur));
        cur = tmp;
        macro_mode = true;

        match_reserved(PP_DEFINED);
        match_reserved(PP_IFNDEF);
        match_reserved(PP_IFDEF);
        match_reserved(PP_IF);
        match_reserved(PP_ELIF);
        match_reserved(PP_ELSE);
        match_reserved(PP_DEF);
        match_reserved(PP_PRAGMA);
        match_reserved(PP_ENDIF);
        {  // PP_INCLUDE
          const char* tmp;
          if (match_strict(cur, &tmp, S_PP_INCLUDE, sizeof(S_PP_INCLUDE) - 1)) {
            tk = Vec_push(tks, Token_new(ID_PP_INCLUDE, cur));
            cur = tmp;
            macro_mode = ID_PP_INCLUDE;
            continue;
          }
        }
        match_reserved(PP_UNDEF);

        continue;
      }
    }
    {
      const char* tmp;
      long x = strntol(cur, &tmp, 10, 128);
      if (*tmp == '.') ERROR(cur, "Not support float");
      if (tmp != cur) {
        tk = Vec_push(tks, Token_new(ID_CONST_INT, cur));
        tk->const_int = x;

        cur = tmp;
        continue;
      }
    }
    {
      // const char* tmp;
      // double x = strntod(cur, &tmp, 128);
      // if (tmp != cur) {
      //   tk = Vec_push(tks, Token_new(ID_CONST_FLOAT, cur));
      //   tk->const_float = x;
      //   cur = tmp;
      //   continue;
      // }
    }
    if (*cur == '\'') {
      const char* tmp = next(cur);
      int c = read_char(tmp, &tmp);
      assert(tmp != next(cur));
      if (*tmp != '\'') ERROR(tmp, "expected \'\\\'\' ");

      tk = Vec_push(tks, Token_new(ID_CONST_CHAR, cur));
      tk->const_int = c;

      cur = next(tmp);
      continue;
    }
    if (*cur == '\"') {
      int len = 0;
      const char* tmp = next(cur);
      while (*tmp != '\"' && *tmp != '\0') {
        if (*tmp == '\\') {
          tmp = next(tmp);
          len++;
        }
        tmp = next(tmp);
        len++;
      }
      if (*tmp == '\0') ERROR(cur, "this should be closed");
      char* str = malloc(len + 1);
      copy_n(str, next(cur), len), str[len] = '\0';

      tk = Vec_push(tks, Token_new(ID_STR, cur));
      tk->corrected = Dict_push(&context->dict, str);

      cur = next(tmp);
      continue;
    }
    if (is_alpha(*cur) || *cur == '_') {
      char ident[MAX_IDENT + 1];
      int len = 0;
      const char* tmp = cur;
      while (is_digit(*tmp) || is_alpha(*tmp) || *tmp == '_') {
        ident[len] = *tmp;
        tmp = next(tmp);
        len++;
      }
      ident[len] = '\0';
      assert(len <= MAX_IDENT);

      tk = Vec_push(tks, Token_new(ID_IDENT, cur));
      tk->corrected = Dict_push_copy(&context->dict, ident);

      cur = tmp;
      continue;
    }

    ERROR(cur, "Internal Error: cannot tokenize");
  }

  tk = Vec_push(tks, Token_new(ID_EOF, cur));
  return tks;
}
#undef match_symbol
#undef match_reserved
