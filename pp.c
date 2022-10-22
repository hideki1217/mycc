#include <string.h>
#include <stdlib.h>

#include "mycc.h"

TokenList* expand(Context* context, Token* input) {
  TokenList* tks = TokenList_new();

  while(!tkislast(input)) {
    Token* tk = TokenList_push(tks);
    memcpy(tk, input, sizeof(Token));
    input++;
  }

  Token* tk = TokenList_push(tks);
  memcpy(tk, input, sizeof(Token));
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
        for(int i=0; i<c; i++) { len_sum += strlen(input[i].corrected); }
        char* combined = malloc(len_sum + 1);
        combined[0] = '\0';
        for(int i=0; i<c; i++) strcat(combined, input[i].corrected);
        
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