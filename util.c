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
  FILE* f = fopen(path, "r");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    s = malloc(length + 1);
    if (s) {
      fread(s, 1, length, f);
      s[length] = '\0';
    }
    fclose(f);
  }

  return s;
}

#define tk2s_helper(name_) \
  if (tk->id == ID_##name_) Buf_writes(buf, S_##name_);
Buf* tk2s(Token* tk, Buf* buf) {
  tk2s_helper(ADD);
  tk2s_helper(SUB);
  tk2s_helper(MUL);
  tk2s_helper(DIV);
  tk2s_helper(MOD);

  tk2s_helper(NOT);
  tk2s_helper(AND);
  tk2s_helper(OR);
  tk2s_helper(XOR);
  tk2s_helper(USHFT);
  tk2s_helper(DSHFT);
  tk2s_helper(INSERT);
  tk2s_helper(QUEST);

  tk2s_helper(ADD_ASGN);
  tk2s_helper(SUB_ASGN);
  tk2s_helper(MUL_ASGN);
  tk2s_helper(DIV_ASGN);
  tk2s_helper(MOD_ASGN);
  tk2s_helper(AND_ASGN);
  tk2s_helper(OR_ASGN);
  tk2s_helper(XOR_ASGN);
  tk2s_helper(USHFT_ASGN);
  tk2s_helper(DSHFT_ASGN);

  tk2s_helper(LESS_THN);
  tk2s_helper(GRTR_THN);
  tk2s_helper(OR_LESS);
  tk2s_helper(OR_GRTR);
  tk2s_helper(EQL);
  tk2s_helper(NOT_EQL);

  tk2s_helper(LGNOT);
  tk2s_helper(LGAND);
  tk2s_helper(LGOR);

  tk2s_helper(L0);
  tk2s_helper(R0);
  tk2s_helper(L1);
  tk2s_helper(R1);
  tk2s_helper(L2);
  tk2s_helper(R2);

  tk2s_helper(C0);
  tk2s_helper(C1);
  tk2s_helper(C2);
  tk2s_helper(C);
  tk2s_helper(CCC);
  tk2s_helper(ARROW);

  tk2s_helper(IF);
  tk2s_helper(ELSE);
  tk2s_helper(DO);
  tk2s_helper(GOTO);
  tk2s_helper(SWITCH);
  tk2s_helper(WHILE);
  tk2s_helper(FOR);

  tk2s_helper(CONTNE);
  tk2s_helper(BREAK);
  tk2s_helper(RETURN);

  tk2s_helper(CASE);
  tk2s_helper(DFLT);

  tk2s_helper(STRCT);
  tk2s_helper(UNION);
  tk2s_helper(ENUM);

  tk2s_helper(VOID);
  tk2s_helper(CHAR);
  tk2s_helper(SHRT);
  tk2s_helper(INT);
  tk2s_helper(LONG);

  tk2s_helper(FLOAT);
  tk2s_helper(DOBLE);

  tk2s_helper(TYPEDEF);
  tk2s_helper(STATIC);
  tk2s_helper(EXTERN);

  tk2s_helper(CONST);
  tk2s_helper(VLTLE);

  tk2s_helper(PP_SYMBL);

  tk2s_helper(PP_CONCAT);
  tk2s_helper(PP_DEF);
  tk2s_helper(PP_IF);
  tk2s_helper(PP_IFDEF);
  tk2s_helper(PP_IFNDEF);
  tk2s_helper(PP_ELIF);
  tk2s_helper(PP_ELSE);
  tk2s_helper(PP_ENDIF);
  tk2s_helper(PP_PRAGMA);
  tk2s_helper(PP_UNDEF);
  tk2s_helper(PP_INCLUDE);
  tk2s_helper(PP_DEFINED);

  if (tk->id == ID_PP_END || tk->id == ID_EOF) {
    // nothing
  }
  if (tk->id == ID_STR) {
    Buf_writec(buf, '\"');
    Buf_writes(buf, tk->corrected);
    Buf_writec(buf, '\"');
  }
  if (tk->id == ID_IDENT) {
    Buf_writes(buf, tk->corrected);
  }
  if (tk->id == ID_PP_INCLUDE_PATH) {
    Buf_writec(buf, '<');
    Buf_writes(buf, tk->corrected);
    Buf_writec(buf, '>');
  }
  if (tk->id == ID_CONST_CHAR) {
    Buf_writec(buf, '\'');
    Buf_writec(buf, tk->const_int);
    Buf_writec(buf, '\'');
  }
  if (tk->id == ID_CONST_INT) {
    char b[32];
    sprintf(b, "%ld", tk->const_int);
    Buf_writes(buf, b);
  }

  return buf;
}
#undef tk2s_helper
