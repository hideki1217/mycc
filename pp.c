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

static bool match(const Token* input, const Token** ls, IDs id) {
  if (input->id == id) {
    *ls = ++input;
    return true;
  }
  return false;
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

void _expand(Context* context, const Token* input, Macros* macros,
             TokenList* output);

void expand_include(Context* context, const Token* args, Macros* macros,
                    TokenList* output) {
  assert(args->id == ID_STR || args->id == ID_PP_INCLUDE_PATH);
  assert(args[1].id == ID_PP_END);

  const char* path =
      find_include_path(context, args->corrected, (args->id == ID_STR));
  assert(path != NULL);
  const char* content = read_text(path);
  assert(content != NULL);

  // マクロ情報だけ継承して更に展開
  Context ctx = {path, content, context->dict};
  TokenList* tokenized = tokenize(&ctx);
  _expand(&ctx, tokenized->buf, macros, output);

  free((void*)content);
  free((void*)path);
}

void _expand(Context* context, const Token* input, Macros* macros,
             TokenList* output) {
  while (!tkislast(input)) {
    if (match(input, &input, ID_PP_SYMBL)) {
      if (match(input, &input, ID_PP_INCLUDE)) {
        expand_include(context, input, macros, output);
        continue;
      }
      if (match(input, &input, ID_PP_END)) {
        continue;
      }
    }

    Token* tk = TokenList_push(output);
    memcpy(tk, input, sizeof(Token));
    input++;
  }

  Token* tk = TokenList_push(output);
  memcpy(tk, input, sizeof(Token));
}

TokenList* expand(Context* context, Token* input) {
  TokenList* tks = TokenList_new();
  Macros macros = Macros_new();
  _expand(context, input, &macros, tks);
  return tks;
}

TokenList* conbine_str(Context* context, Token* input) {
  TokenList* tks = TokenList_new();

  while (!tkislast(input)) {
    if (input->id == ID_STR) {
      int c = 0;
      for (Token* tmp = input; tmp->id == ID_STR; tmp++) {
        c++;
      }
      if (c > 1) {
        int len_sum = 0;
        for (int i = 0; i < c; i++) {
          len_sum += strlen(input[i].corrected);
        }
        char* combined = malloc(len_sum + 1);
        combined[0] = '\0';
        for (int i = 0; i < c; i++) strcat(combined, input[i].corrected);

        Token* tk = TokenList_push(tks);
        tk->id = ID_STR;
        tk->pos = input->pos;
        tk->corrected = Dict_push(context->dict, combined);

        input = input + c;
        continue;
      }
    }
    Token* tk = TokenList_push(tks);
    memcpy(tk, input, sizeof(Token));
    input++;
  }

  Token* tk = TokenList_push(tks);
  memcpy(tk, input, sizeof(Token));
  return tks;
}

TokenList* preprocess(Context* context, Token* input) {
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
  TokenList* expanded = expand(context, input);
  TokenList* combined = conbine_str(context, expanded->buf);
  return combined;
}