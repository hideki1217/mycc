#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"

int main() {
  const char* path = "/home/hideki/Documents/repositories/mycc/tests/test_tokenize.txt";

  char* s = read_text(path);

  if (s) {
    Context context = {path, s, Dict_new()};
    TokenList* res = tokenize(&context);

    for(Token* cur = res->buf; !tkislast(cur); cur++ ) {
      if(cur->id == ID_STR || cur->id == ID_IDENT || cur->id == ID_PP_INCLUDE_PATH) {
        // int x, y;
        // position_info(s, cur->pos, &x, &y);
        // printf("%s:%d:%d: ", path, y, x);
        printf("%ld: %s\n", (long)cur->corrected, cur->corrected);
      }
    }

    Dict_free(context.dict);
  }
  else {
    printf("file is not found");
    return 1;
  }
}