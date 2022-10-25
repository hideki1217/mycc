#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"


int main() {
  const char* path = "/home/hideki/Documents/repositories/mycc/tests/test_pp.txt";

  const char* s = read_text(path);

  if (s) {
    Context context = {path, s, Dict_new(), StrV_new()};
    {
      *StrV_push(context.include_path) = "/usr/include";
    }
    TokenList* tokenized = tokenize(&context);
    TokenList* pped = preprocess(&context, tokenized->buf);

    for(Token* cur = pped->buf; !tkislast(cur); cur++ ) {
      if(cur->id == ID_STR || cur->id == ID_IDENT || cur->id == ID_PP_INCLUDE_PATH) {
        // int x, y;
        // position_info(s, cur->pos, &x, &y);
        // printf("%s:%d:%d: ", path, y, x);
        printf("%ld: %s\n", (long)cur->corrected, cur->corrected);
      }
    }

    Dict_free(context.dict);
    StrV_free(context.include_path);
  }
  else {
    printf("file is not found");
    return 1;
  }
}