#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

typedef struct {
  Vec* ts;    // null => symbol macro
  Vec* args;  // null ? ()-less macro : ()d macro
} Macro;
#define macro_is_func(macro_) ((macro_)->args != NULL)
typedef struct {
  Map t;
} Macros;
Macros Macros_new() {
  Macros res = {Map_new()};
  return res;
}
bool Macros_push(Macros* self, long key, Macro* item) {
  return Map_push(&self->t, key, item);
}
Macro* Macros_pushf(Macros* self, long key, Macro* item) {
  return Map_pushf(&self->t, key, item);
}
Macro* Macros_contain(Macros self, long key) {
  return Map_contain(self.t, key);
}
Macro* Macros_delete(Macros* self, long key) {
  return Map_delete(&self->t, key);
}
void Macros_free(Macros self) { Map_free(self.t); }

static Macros macros;
static Vec* contexts = NULL;
#define context ((Context*)(contexts->buf[contexts->len - 1]))

static const char* _find_pathn(const char* base_dir, const char* path, int n) {
  if (n <= 0) return NULL;
  DIR* dir;
  struct dirent* dp;
  char target[MAX_PATH];
  char x[MAX_PATH];
  const char* res = NULL;

  strcpy(target, base_dir);
  pathcat(target, path);

  dir = opendir(base_dir);
  if (dir != NULL) {
    for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
        continue;
      strcpy(x, base_dir);
      pathcat(x, dp->d_name);
      if (dp->d_type == DT_DIR) {
        res = _find_pathn(x, path, n - 1);
        if (res) break;
      } else {
        strcpy(x, base_dir);
        pathcat(x, dp->d_name);
        if (strcmp(target, x) == 0) {
          res = malloc(strlen(target) + 1);
          strcpy((char*)res, target);
          break;
        }
      }
    }
  } else {
    abort();
  }

  closedir(dir);
  return res;
}

static const char* find_include_path(const char* path, bool local) {
  DIR* dir;
  struct dirent* dp;
  const char* res = NULL;
  char base_dir[MAX_PATH];
  char target[MAX_PATH];
  char x[MAX_PATH];

  if (local) {
    pathparen(base_dir, context->name);
    res = _find_pathn(base_dir, path, 5);
    if (res) return res;
  }

  for (int i = 0; i < context->include_path->len; i++) {
    pathnorm(base_dir, context->include_path->buf[i]);
    res = _find_pathn(base_dir, path, MAX_INCLUDE_DEPTH);
    if (res) return res;
  }

  abort();
}

#define CHECK_NTH(tks_, id_, x_) \
  (((tks_)->len <= (x_))         \
       ? false                   \
       : (((Token**)((tks_)->buf))[(tks_)->len - 1 - (x_)]->id == id_))
#define CHECK(tks_, id_) CHECK_NTH(tks_, id_, 0)
#define CHECK2(tks_, id0_, id1_)                                    \
  (((tks_)->len < 2)                                                \
       ? false                                                      \
       : (((Token**)((tks_)->buf))[(tks_)->len - 1]->id == id0_) && \
             (((Token**)((tks_)->buf))[(tks_)->len - 2]->id == id1_))
static bool match(const Token* input, const Token** ls, IDs id) {
  if (input->id == id) {
    *ls = ++input;
    return true;
  }
  return false;
}

static int tkslen(Vec* tks) { return tks->len; }
static Token* tksget(Vec* tks, int idx) { return Vec_get(tks, idx); }
static Token* tkslast(Vec* tks) { return Vec_get(tks, tks->len - 1); }
static Token* tkspop(Vec* tks) { return Vec_pop(tks); }
static Token* tkspush(Vec* tks, Token* x) { return Vec_push(tks, x); }
static Vec* tkscpy(Vec* ts) {
  Vec* res = Vec_withsize(ts->len);
  for (int i = 0; i < ts->len; i++) Vec_push(res, ts->buf[i]);
  return res;
}

static Vec* take_until(Vec* tks, IDs id) {
  Vec* res = Vec_new();
  while (!CHECK(tks, id) && !Vec_empty(tks)) {
    tkspush(res, tkspop(tks));
  }
  assert(CHECK(tks, id));
  return res;
}

static void _expand(Vec* input_r, Vec* output);

static void expand_include(Vec* input_r, Vec* output) {
  Token* arg = tkspop(input_r);
  assert(arg->id == ID_STR || arg->id == ID_PP_INCLUDE_PATH);
  Token* pp_end = tkspop(input_r);
  assert(pp_end->id == ID_PP_END);

  const char* path = find_include_path(arg->corrected, (arg->id == ID_STR));
  assert(path != NULL);
  const char* content = read_text(path);
  assert(content != NULL);

  // マクロ情報だけ継承して更に展開
  Context ctx = {path, content, context->dict, context->include_path};
  Vec* tokenized = tokenize(&ctx);

  Token* eof = tkspop(tokenized);
  assert(eof->id == ID_EOF);
  Token_free(eof);

  Vec_push(contexts, &ctx);
  _expand(Vec_reverse(tokenized), output);
  Vec_pop(contexts);

  free((void*)content);
  free((void*)path);
  Token_free(arg);
  Token_free(pp_end);
}

static void add_macro(Vec* input_r) {
  assert(CHECK(input_r, ID_IDENT));

  Token* name = tkspop(input_r);
  Vec* args = NULL;
  Vec* ts = NULL;
  if (CHECK(input_r, ID_L0)) {
    // functional
    Token_free(tkspop(input_r));

    args = Vec_new();
    /* ((IDENT,)*(IDENT(...)? | ...)?) */
    while (CHECK2(input_r, ID_IDENT, ID_C0)) {
      tkspush(args, tkspop(input_r));
      Token_free(tkspop(input_r));
    }
    if (CHECK2(input_r, ID_IDENT, ID_CCC)) {
      tkspush(args, tkspop(input_r));
      tkspush(args, tkspop(input_r));
    } else if (CHECK(input_r, ID_IDENT)) {
      tkspush(args, tkspop(input_r));
    } else if (CHECK(input_r, ID_CCC)) {
      Token* ccc = tkspop(input_r);
      Token* ident = Token_new(ID_IDENT, ccc->pos);
      ident->corrected = Dict_push_copy(&context->dict, S_VA_ARGS);

      tkspush(args, ident);
      tkspush(args, ccc);
    } else
      abort();

    assert(CHECK(input_r, ID_R0));
    Token_free(tkspop(input_r));
  }

  while (!CHECK(input_r, ID_PP_END)) {
    if (ts == NULL) ts = Vec_new();
    tkspush(ts, tkspop(input_r));
  }
  Token_free(tkspop(input_r));  // ID_PP_END

  Macro* macro = malloc(sizeof(Macro));
  macro->args = args;
  macro->ts = ts;

  assert(Macros_push(&macros, (long)name->corrected, macro));
}

static void jump_to_else_elif_endif(Vec* input_r) {
  int if_count = 0;
  while (!Vec_empty(input_r)) {
    if (CHECK(input_r, ID_PP_IF) || CHECK(input_r, ID_PP_IFDEF) ||
        CHECK(input_r, ID_PP_IFNDEF)) {
      if_count++;
    }
    if (CHECK(input_r, ID_PP_ENDIF) || CHECK(input_r, ID_PP_ELSE) ||
        CHECK(input_r, ID_PP_ELIF)) {
      if (if_count == 0) {
        break;
      }
    }
    if (CHECK(input_r, ID_PP_ENDIF) && if_count != 0) if_count--;
    assert(if_count >= 0);
    Token_free(tkspop(input_r));
  }
  assert(CHECK(input_r, ID_PP_ENDIF) || CHECK(input_r, ID_PP_ELSE) ||
         CHECK(input_r, ID_PP_ELIF));
}

static void jump_to_endif(Vec* input_r) {
  int if_count = 0;
  while (!Vec_empty(input_r)) {
    if (CHECK(input_r, ID_PP_IF) || CHECK(input_r, ID_PP_IFDEF) ||
        CHECK(input_r, ID_PP_IFNDEF)) {
      if_count++;
    }
    if (CHECK(input_r, ID_PP_ENDIF)) {
      if (if_count != 0)
        if_count--;
      else
        break;
    }
    assert(if_count >= 0);
    Token_free(tkspop(input_r));
  }
  assert(CHECK(input_r, ID_PP_ENDIF));
}

static int eval_if(Vec* input_r) {
  Vec* content = take_until(input_r, ID_PP_END);
  Vec* expr = Vec_new();
  _expand(content, expr);

  TODO();
  return 1;
}

static int if_jump(Vec* input_r, bool cond) {
  if (cond) {
    return 1;
  } else {
    while (true) {
      jump_to_else_elif_endif(input_r);
      Token* key = tkspop(input_r);
      if (key->id == ID_PP_ELSE) {
        Token_free(key);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));

        return 1;
      }
      if (key->id == ID_PP_ENDIF) {
        Token_free(key);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));

        return 0;
      }
      if (key->id == ID_PP_ELIF) {
        Token_free(key);
        bool cond = eval_if(input_r);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));

        if (cond) {
          return 1;
        }
      }
    }
  }
}

static void hsadd(Vec* ts, Set hs) {
  for (int i = 0; i < ts->len; i++)
    Set_union_asgn(&((Token*)ts->buf[i])->hideset, hs);
}

static int search_arg(Vec* fp, Token* tk, int* elipsis) {
  *elipsis = false;
  if (fp == NULL) return -1;
  assert(tk->id == ID_IDENT);
  for (int i = 0; i < fp->len; i++) {
    Token* x = tksget(fp, i);
    if (x->id == ID_CCC) break;
    if (x->corrected == tk->corrected) {
      *elipsis = (i + 1 < fp->len) ? tksget(fp, i + 1)->id == ID_CCC : false;
      return i;
    }
  }
  return -1;
}

static Vec* select_arg(Vec* ap, int idx, int elipsis) {
  static Vec* res = NULL;
  if (res == NULL)
    res = Vec_new();
  else
    Vec_clear(res);

  int i = 0;
  for (int n = 0; n < idx; i++) {
    if (tksget(ap, i)->id == ID_C0) n++;
  }
  if (elipsis) {
    for (; i < ap->len; i++) tkspush(res, Token_cpy(tksget(ap, i)));

  } else {
    for (; i < ap->len; i++) {
      if (tksget(ap, i)->id == ID_C0) break;

      tkspush(res, Token_cpy(tksget(ap, i)));
    }
  }

  return res;
}

static Token* tkconcat(Token* lhs, Token* rhs) {
  assert(lhs->id == ID_IDENT);
  assert(rhs->id == ID_IDENT);

  int len = strlen(lhs->corrected) + strlen(rhs->corrected);

  Token* x = Token_new(ID_IDENT, lhs->pos);
  x->corrected = Dict_push(
      &context->dict,
      strcat(strcpy(malloc(len + 1), lhs->corrected), rhs->corrected));

  return x;
}

static Vec* subst(Vec* ts, Vec* fp, Vec* ap, Set hs) {
  static Vec* res = NULL;
  if (res == NULL) res = Vec_new();

  if (ts == NULL) return res;

  int mode = 0;
  int empty_replace = 0;
  for (int i = 0; i < ts->len; i++) {
    Token* tk = tksget(ts, i);

    if (mode == ID_PP_SYMBL) {
      mode = 0;

      if (tk->id == ID_IDENT) {
        int elipsis;
        int idx = search_arg(fp, tk, &elipsis);
        if (idx >= 0) {
          Vec* ref = select_arg(ap, idx, elipsis);

          Buf* buf = Buf_new();
          for (int i = 0; i < ref->len; i++) {
            tk2s(tksget(ref, i), buf);
          }

          Token* x = Token_new(ID_STR, tk->pos);
          x->corrected = Dict_push(&context->dict, Buf_into_str(buf));
          tkspush(res, x);

          empty_replace = 0;
          continue;
        }
      }
      abort();
    }

    if (mode == ID_PP_CONCAT) {
      mode = 0;

      if (!empty_replace) {
        assert(res->len > 0);
        Token* l_ident = Vec_pop(res);
        assert(l_ident->id == ID_IDENT);
        assert(tk->id == ID_IDENT);

        int elipsis;
        int idx = search_arg(fp, tk, &elipsis);
        if (idx >= 0) {
          Vec* ref = select_arg(ap, idx, elipsis);
          if (ref->len != 0) {
            Token* x = tkconcat(l_ident, tksget(ref, 0));

            tkspush(res, x);
            for (int i = 1; i < ref->len; i++) {
              tkspush(res, tksget(ref, i));
            }
          }
        } else {
          Token* x = tkconcat(l_ident, tk);
          tkspush(res, x);
        }

        Token_free(l_ident);
        empty_replace = 0;
        continue;
      }
    }

    if (tk->id == ID_PP_SYMBL) {
      assert(mode == 0);
      mode = ID_PP_SYMBL;
      empty_replace = 0;
      continue;
    }
    if (tk->id == ID_PP_CONCAT) {
      assert(mode == 0);
      mode = ID_PP_CONCAT;
      empty_replace = 0;
      continue;
    }

    if (tk->id == ID_IDENT) {
      int elipsis;
      int idx = search_arg(fp, tk, &elipsis);
      if (idx >= 0) {
        Vec* ref = select_arg(ap, idx, elipsis);
        if (ref->len == 0) {
          empty_replace = 1;
          continue;
        } else {
          _expand(Vec_reverse(ref), res);
          empty_replace = 0;
          continue;
        }
      }
    }
    tkspush(res, Token_cpy(tk));
  }

  hsadd(res, hs);

  Set_free(hs);
  if (ap) {
    for (int i = 0, len = ap->len; i < len; i++) {
      Token_free(tkspop(ap));
    }
    Vec_free(ap);
  }

  return res;
}

static bool check_args(Macro* macro, Vec* ap) {
  Vec* fp = macro->args;
  int elipsis = tkslast(fp)->id == ID_CCC;
  int arg_n = fp->len - elipsis;

  int n = 1;
  for (int ap_i = 0; ap_i < ap->len; ap_i++) {
    Token* a = tksget(ap, ap_i);
    if (a->id == ID_C0) n++;
  }

  if (n == arg_n) return true;
  if (n > arg_n && elipsis) return true;
  return false;
}

static void _expand(Vec* input_r, Vec* output) {
  int if_count = 0;
  while (!Vec_empty(input_r)) {
    if (CHECK(input_r, ID_PP_SYMBL)) {
      Token_free(tkspop(input_r));

      if (CHECK(input_r, ID_PP_INCLUDE)) {
        Token_free(tkspop(input_r));

        expand_include(input_r, output);
        continue;
      }
      if (CHECK(input_r, ID_PP_DEF)) {
        Token_free(tkspop(input_r));

        add_macro(input_r);
        continue;
      }
      if (CHECK(input_r, ID_PP_IF)) {
        Token_free(tkspop(input_r));

        bool cond = eval_if(input_r);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));  // ID_PP_END

        if_count += if_jump(input_r, cond);
        continue;
      }
      if (CHECK(input_r, ID_PP_IFDEF)) {
        Token_free(tkspop(input_r));

        assert(CHECK(input_r, ID_IDENT));
        Token* ident = tkspop(input_r);
        bool cond = Macros_contain(macros, (long)ident->corrected) != NULL;

        Token_free(ident);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));  // ID_PP_END

        if_count += if_jump(input_r, cond);
        continue;
      }
      if (CHECK(input_r, ID_PP_IFNDEF)) {
        Token_free(tkspop(input_r));

        assert(CHECK(input_r, ID_IDENT));
        Token* ident = tkspop(input_r);
        bool cond = Macros_contain(macros, (long)ident->corrected) == NULL;

        Token_free(ident);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));  // ID_PP_END

        if_count += if_jump(input_r, cond);
        continue;
      }
      if (CHECK(input_r, ID_PP_ELSE) || CHECK(input_r, ID_PP_ELIF)) {
        Token_free(tkspop(input_r));

        assert(if_count > 0);
        if_count--;

        jump_to_endif(input_r);
        assert(CHECK(input_r, ID_PP_ENDIF));
        Token_free(tkspop(input_r));
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));

        continue;
      }
      if (CHECK(input_r, ID_PP_ENDIF)) {
        Token_free(tkspop(input_r));

        assert(if_count > 0);
        if_count--;

        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));

        continue;
      }
      if (CHECK(input_r, ID_PP_PRAGMA)) {
        Token_free(tkspop(input_r));
        // TODO: 今はすべて無視
        while (!CHECK(input_r, ID_PP_END)) Token_free(tkspop(input_r));
        Token_free(tkspop(input_r));
        continue;
      }
      if (CHECK(input_r, ID_PP_UNDEF)) {
        Token_free(tkspop(input_r));
        assert(CHECK(input_r, ID_IDENT));
        Token* ident = tkspop(input_r);

        Macro* macro = Macros_delete(&macros, (long)ident->corrected);
        if (macro) free(macro);

        Token_free(ident);
        assert(CHECK(input_r, ID_PP_END));
        Token_free(tkspop(input_r));
        continue;
      }
      if (CHECK(input_r, ID_PP_END)) {
        Token_free(tkspop(input_r));
        continue;
      }
      abort();
    }

    Token* tk = tkspop(input_r);
    if (tk->id != ID_IDENT || Set_contain(tk->hideset, (long)tk->corrected)) {
      tkspush(output, tk);
      continue;
    }
    Macro* macro = Macros_contain(macros, (long)tk->corrected);
    if (macro == NULL) {
      tkspush(output, tk);
      continue;
    }
    Vec* expnded;
    if (macro_is_func(macro)) {
      if (!CHECK(input_r, ID_L0)) {
        tkspush(output, tk);
        continue;
      }
      Token_free(tkspop(input_r));

      Vec* actuals = take_until(input_r, ID_R0);
      assert(check_args(macro, actuals));
      assert(CHECK(input_r, ID_R0));
      Token* r0 = tkspop(input_r);

      Set hs = Set_cross(tk->hideset, r0->hideset);
      hs = Set_push(&hs, (long)tk->corrected);
      expnded = subst(macro->ts, macro->args, actuals, hs);
      Token_free(r0);
    } else {
      Set hs = Set_cpy(tk->hideset);
      hs = Set_push(&hs, (long)tk->corrected);
      expnded = subst(macro->ts, NULL, NULL, hs);
    }
    for (int i = 0, n = expnded->len; i < n; i++)
      tkspush(input_r, Vec_pop(expnded));
    Token_free(tk);
  }
  assert(if_count == 0);
}

static Vec* expand(Vec* inputs) {
  Vec* outputs = Vec_new();
  macros = Macros_new();

  _expand(Vec_reverse(inputs), outputs);

  Macros_free(macros);

  return outputs;
}

static Vec* conbine_str(Vec* inputs) {
  Vec* outputs = Vec_new();
  tkspush(inputs, NULL);  // iterator の終了判定

  Token** input = (Token**)inputs->buf;
  while (*input != NULL) {
    if ((*input)->id == ID_STR) {
      int c = 0;
      for (Token** tmp = input; (*tmp)->id == ID_STR; tmp++) {
        c++;
      }
      if (c > 1) {
        int len_sum = 0;
        for (int i = 0; i < c; i++) {
          len_sum += strlen(input[i]->corrected);
        }
        char* combined = malloc(len_sum + 1);
        combined[0] = '\0';
        for (int i = 0; i < c; i++) strcat(combined, input[i]->corrected);

        Token* tk = tkspush(outputs, Token_new(ID_STR, (*input)->pos));
        tk->corrected = Dict_push(&context->dict, combined);

        for (int i = 0; i < c; i++) Token_free(input[i]);

        input = input + c;
        continue;
      }
    }
    Token* tk = tkspush(outputs, *input);
    input++;
  }
  return outputs;
}

static void remove_hs(Vec* tks) {
  for (int i = 0; i < tks->len; i++) {
    Token* tk = tksget(tks, i);

    Set_free(tk->hideset);
    tk->hideset = NULL;
  }
}

Vec* preprocess(Context* ctx, Vec* input) {
  //前から読んでいきながら、Macroデータベースを更新
  // includeを見つけたら展開し、展開後のはじめからスタート
  // ifdef, ifndef
  // を見つけたら、以降の最初のトークンを識別子とみなして、それがデータベースにあるか確認(ここではマクロ展開はしない)
  //あれば、endifまでを採用し、else以内を消す。そして生きてるとこの先頭からスタート
  // if
  // を見つけたら、以降のトークンをプリプロセス、そしてプリプロセス構文に基づいてパースし、評価。評価に失敗したらエラー。成功したら、
  //その値に応じて、上と同じ操作
  // pragmaを見つけたら、とりあえず無視。
  // undef を見たら、最初のトークンをデータベースから削除。なくてもいい
  // define
  // を見たら、最初のトークンは識別子、データベースにあればワーニングして上書き。
  // IDENT(　のときは関数マクロなので、関数マクロ登録の処理
  // IDENT　のときは値マクロなので、値マクロ登録の処理
  //データベースにある識別子を見つけた^呼び出し可能になっている(正当な引数がついている)ならば
  //マクロ展開
  if (contexts == NULL) contexts = Vec_new();

  Vec_push(contexts, ctx);

  Vec* expanded = expand(input);
  Vec* combined = conbine_str(expanded);
  remove_hs(combined);

  Vec_pop(contexts);

  return combined;
}