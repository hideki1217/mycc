#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "collection.h"
#include "mycc.h"

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

static const char* find_include_path(Context* context, const char* path,
                                     bool local) {
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

#define POP(tks) TokenS_pop(tks)
#define PUSH(tks, tk) TokenS_push(tks, tk)
static bool match(const Token* input, const Token** ls, IDs id) {
  if (input->id == id) {
    *ls = ++input;
    return true;
  }
  return false;
}

static void reverse(void** buf, int n) {
  for (int i = 0; i < n / 2; i++) {
    void* tmp = buf[i];
    buf[i] = buf[n - 1 - i];
    buf[n - 1 - i] = tmp;
  }
}

static TokenS* TokenS_reverse(TokenS* tks) {
  reverse((void**)tks->buf, tks->len);
  return tks;
}

typedef struct {
  Token* ts;
  Token* args;
} Macro;
typedef struct {
  AvlTree t;
} Macros;
Macros Macros_new() {
  Macros res = {AVL_EMPTY};
  return res;
}
bool Macros_push(Macros* self, int key, Macro* item) {
  return Avl_push(&self->t, key, item);
}
void* Macros_pushf(Macros* self, int key, Macro* item) {
  return Avl_pushf(&self->t, key, item);
}
Macro* Macros_contain(Macros self, int key) { return Avl_contain(self.t, key); }
void Macros_free(Macros self) { Avl_free(self.t); }

void _expand(Context* context, TokenS* input_r, Macros* macros, TokenS* output);

void expand_include(Context* context, TokenS* input_r, Macros* macros,
                    TokenS* output) {
  Token* arg = POP(input_r);
  assert(arg->id == ID_STR || arg->id == ID_PP_INCLUDE_PATH);
  Token* pp_end = POP(input_r);
  assert(pp_end->id == ID_PP_END);

  const char* path =
      find_include_path(context, arg->corrected, (arg->id == ID_STR));
  assert(path != NULL);
  const char* content = read_text(path);
  assert(content != NULL);

  // マクロ情報だけ継承して更に展開
  Context ctx = {path, content, context->dict};
  TokenS* tokenized = tokenize(&ctx);
  _expand(&ctx, TokenS_reverse(tokenized), macros, output);

  free((void*)content);
  free((void*)path);
  free(arg);
  free(pp_end);
}

void _expand(Context* context, TokenS* input_r, Macros* macros,
             TokenS* output) {
  while (!TokenS_empty(input_r)) {
    Token* input = POP(input_r);
    if (input->id == ID_PP_SYMBL) {
      Token* key = POP(input_r);
      if (key->id == ID_PP_INCLUDE) {
        expand_include(context, input_r, macros, output);

        free(input);
        free(key);
        continue;
      }
      if (key->id == ID_PP_DEF) {
        abort();
        free(input);
        free(key);
        continue;
      }
      if (key->id == ID_PP_IF) {
        abort();
        free(input);
        free(key);
        continue;
      }
      if (key->id == ID_PP_IFDEF) {
        abort();
        free(input);
        free(key);
        continue;
      }
      if (key->id == ID_PP_IFNDEF) {
        abort();
        free(input);
        free(key);
        continue;
      }
      if (key->id == ID_PP_END) {
        free(input);
        free(key);
        continue;
      }
      abort();
    }

    TokenS_push(output, input);
  }
}

TokenS* expand(Context* context, TokenS* inputs) {
  TokenS* outputs = TokenS_new();
  Macros macros = Macros_new();

  _expand(context, TokenS_reverse(inputs), &macros, outputs);

  return outputs;
}

TokenS* conbine_str(Context* context, TokenS* inputs) {
  TokenS* outputs = TokenS_new();
  TokenS_push(inputs, NULL);  // iterator の終了判定

  Token** input = inputs->buf;
  while (*input != NULL) {
    if ((*input)->id == ID_STR) {
      int c = 0;
      for (Token* tmp = (*input); tmp->id == ID_STR; tmp++) {
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

        Token* tk = TokenS_push(outputs, Token_new(ID_STR, (*input)->pos));
        tk->corrected = Dict_push(context->dict, combined);

        input = input + c;
        continue;
      }
    }
    Token* tk = TokenS_push(outputs, *input);
    input++;
  }
  return outputs;
}

TokenS* preprocess(Context* context, TokenS* input) {
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
  TokenS* expanded = expand(context, input);
  TokenS* combined = conbine_str(context, expanded);
  return combined;
}