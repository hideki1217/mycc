#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

int main() {
  const char* path =
      "/home/hideki/Documents/repositories/mycc/tests/test_pp_sample.c";
  const char* true_path =
      "/home/hideki/Documents/repositories/mycc/tests/test_pp_true.c";

  const char* s = read_text(path);
  const char* s_true = read_text(true_path);

  if (s && s_true) {
    Context context = {path, s, Dict_new(), Vec_new()};
    { Vec_push(context.include_path, "/usr/include"); }
    Vec* tked = tokenize(&context);
    Vec* pped = preprocess(&context, tked);

    Context context_true = {true_path, s_true, context.dict,
                            context.include_path};
    Vec* tked_true = tokenize(&context_true);

    Buf *buf = Buf_new(), *buf_true = Buf_new();
    for (int i = 0; i < min(pped->len, tked_true->len); i++) {
      Token* cur = Vec_get(pped, i);
      Token* cur_true = Vec_get(tked_true, i);

      Buf_clear(buf), Buf_clear(buf_true);
      tk2s(cur, buf), tk2s(cur_true, buf_true);
      printf("%s :: %s\n", Buf_ref(buf), Buf_ref(buf_true));

      if (cur->id != cur_true->id) {
        printf("%d |actual = %d: true = %d", i, cur->id, cur_true->id);
        abort();
      } else {
        if (cur->id == ID_STR || cur->id == ID_IDENT ||
            cur->id == ID_PP_INCLUDE_PATH) {
          assert(cur->corrected == cur_true->corrected);
        }
      }
    }
    assert(pped->len == tked_true->len);

    Dict_free(context.dict);
    Vec_free(context.include_path);
  } else {
    printf("file is not found");
    return 1;
  }
}