#include "mycc.h"

TokenList* expand(const char* name, const char* content, Token* input) {
  //todo
}


TokenList* conbine_str(const char* name, const char* content, Token* input) {
  //todo
}

TokenList* preprocess(const char* name, const char* content, Token *input) {
  //前から読んでいきながら、Macroデータベースを更新
  //includeを見つけたら展開し、展開後のはじめからスタート
  //ifdef, ifndef を見つけたら、以降の最初のトークンを識別子とみなして、それがデータベースにあるか確認(ここではマクロ展開はしない)
  //あれば、endifまでを採用し、else以内を消す。そして生きてるとこの先頭からスタート
  //if を見つけたら、以降のトークンをプリプロセス、そしてプリプロセス構文に基づいてパースし、評価。評価に失敗したらエラー。成功したら、
  //その値に応じて、上と同じ操作
  //pragmaを見つけたら、とりあえず無視。
  //undef を見たら、最初のトークンをデータベースから削除。なくてもいい
  //define を見たら、最初のトークンは識別子、データベースにあればワーニングして上書き。
  //IDENT(　のときは関数マクロなので、関数マクロ登録の処理
  //IDENT　のときは値マクロなので、値マクロ登録の処理
  //データベースにある識別子を見つけた^呼び出し可能になっている(正当な引数がついている)ならば
  //マクロ展開
  TokenList* expanded = expand(name, content, input);
  TokenList* combined = conbine_str(name, content, expanded->buf);
  return combined;
}