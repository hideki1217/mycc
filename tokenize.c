#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "mycc.h"

Token* Token_new(Token* prev) {
  Token* x = malloc(sizeof(Token));
  x->next = NULL;
  prev->next = x;
  return x;
}

int is_space(char c) { return c == ' ' || c == '\n' || c == '\t'; }
int is_digit(char c) { return '0' <= c && c <= '9'; }
int is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int char2int(char c, int base) {
  assert(0 < base);
  assert(base <= 36);
  if (base <= 10) {
    if ('0' <= c && c < '0' + base) return c - '0';
  }
  else {
    if ('0' <= c && c < '0' + 10) return c - '0';
    if ('a' <= c && c < 'a' + (base - 10)) return c - 'a' + 10;
  }
  return -1; 
}

bool match(const char* s, const char** ls, const char* word, int word_n) {
  if (strncmp(s, word, word_n) == 0) {
    *ls = s + word_n;
    return true;
  }
  return false;
}
bool match_strict(const char* s, const char** ls, const char* word,
                  int word_n) {
  if (strncmp(s, word, word_n) == 0 && is_space(s[word_n])) {
    *ls = s + word_n;
    return true;
  }
  return false;
}

long strntol(const char* s, const char** ls, int base, int n) {
  assert(n >= 2);
  long x = 0;
  int i, val;
  for (i = 0; i < n; i++) {
    if ((val = char2int(s[i], base)) >= 0) {
      x += val;
      x *= base;
    }
    else break;
  }
  *ls = &s[i];
  return x;
}
char read_char(const char* s, const char** ls) {
  if (s[0] == '\\') {
    if (s[1] == 'a') {
      *ls = s + 2;
      return '\a';
    }
    if (s[1] == 'b') {
      *ls = s + 2;
      return '\b';
    }
    if (s[1] == 'f') {
      *ls = s + 2;
      return '\f';
    }
    if (s[1] == 'n') {
      *ls = s + 2;
      return '\n';
    }
    if (s[1] == 'r') {
      *ls = s + 2;
      return '\r';
    }
    if (s[1] == 't') {
      *ls = s + 2;
      return '\t';
    }
    if (s[1] == 'v') {
      *ls = s + 2;
      return '\v';
    }
    if (s[1] == '\\') {
      *ls = s + 2;
      return '\\';
    }
    if (s[1] == '\?') {
      *ls = s + 2;
      return '\?';
    }
    if (s[1] == '\'') {
      *ls = s + 2;
      return '\'';
    }
    if (s[1] == '\"') {
      *ls = s + 2;
      return '\"';
    }
    if (s[1] == 'x') {
      /*16進数２桁*/
      return strntol(s + 2, ls, 16, 2);
    }
    /*8進数3桁*/
    return strntol(s + 1, ls, 8, 3);
  } else {
    *ls = s + 1;
    return s[0];
  }
}

Token* tokenize(const char* content) {
  Token _root;

  const char* cur = content;

  Token* tk = &_root;
  while (cur[0] != '\0') {
    if (is_space(cur[0])) {
      cur++;
      continue;
    }

#define match_symbol(name_)                                   \
  {                                                           \
    const char* tmp;                                          \
    if (match(cur, &tmp, S_##name_, sizeof(S_##name_) - 1)) { \
      tk = Token_new(tk);                                     \
      tk->id = ID_##name_;                                    \
      tk->pos = cur;                                          \
      cur = tmp;                                              \
      continue;                                               \
    }                                                         \
  }
    match_symbol(USHFT_ASGN);
    match_symbol(DSHFT_ASGN);

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
#define match_reserved(name_)                                        \
  {                                                                  \
    const char* tmp;                                                 \
    if (match_strict(cur, &tmp, S_##name_, sizeof(S_##name_) - 1)) { \
      tk = Token_new(tk);                                            \
      tk->id = ID_##name_;                                           \
      tk->pos = cur;                                                 \
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

    {
      char* tmp;
      long x = strtol(cur, &tmp, 10);
      if (tmp != cur && tmp[0] != '.') {
        tk = Token_new(tk);
        tk->id = ID_CONST_INT;
        tk->pos = cur;
        tk->const_int = x;

        cur = tmp;
        continue;
      }
    }
    {
      char* tmp;
      double x = strtod(cur, &tmp);
      if (tmp != cur) {
        tk = Token_new(tk);
        tk->id = ID_CONST_FLOAT;
        tk->pos = cur;
        tk->const_float = x;

        cur = tmp;
        continue;
      }
    }
    if (cur[0] == '\'') {
      const char* tmp;
      int c = read_char(&cur[1], &tmp);
      assert(tmp != cur + 1);
      assert(tmp[0] == '\'');

      tk = Token_new(tk);
      tk->id = ID_CHAR;
      tk->pos = cur;
      tk->const_int = c;

      cur = tmp + 1;
      continue;
    }
    if (cur[0] == '\"') {
      const char* tmp = cur + 1;
      while (*tmp != '\"' && *tmp != '\0') {
        if (*tmp == '\\') {
          tmp++;
        }
        tmp++;
      }
      assert(*tmp != '\0');

      tk = Token_new(tk);
      tk->id = ID_STR;
      tk->pos = cur;
      tk->len = tmp - cur;

      cur = tmp + 1;
      continue;
    }
    if (is_alpha(cur[0]) || cur[0] == '_') {
      const char* tmp = cur + 1;
      while (is_digit(tmp[0]) || is_alpha(tmp[0]) || tmp[0] == '_') {
        tmp++;
      }

      tk = Token_new(tk);
      tk->id = ID_IDENT;
      tk->pos = cur;
      tk->len = tmp - cur;

      cur = tmp;
      continue;
    }

    abort();
  }

  return _root.next;
}
#undef match_symbol
#undef match_reserved
