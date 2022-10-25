#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "mycc.h"

static void print_pos(const char* line, int x, int y) {
  int n;
  printf("%d | %n", y + 1, &n);

  for (int n = 0; line[n] != '\n' && n < 256; n++) {
    printf("%c", line[n]);
  }
  printf("\n");
  for (int i = 0; i < n - 2; i++) printf(" ");
  printf("| ");
  assert(x < 256);
  for (int i = 0; i < x; i++) printf(" ");
  printf("^~~~\n");
}

const char* position_info(const char* content, const char* pos, int* x,
                          int* y) {
  /**
   * @brief get position
   * @return line top address
   */
  assert(content <= pos);
  const char* cur = content;
  int _x = 0, _y = 0;
  const char* line = cur;
  while (cur != pos) {
    if (*cur == '\n') {
      _x = 0;
      _y++;
      line = cur + 1;
    } else {
      _x++;
    }
    cur++;
  }

  *y = _y;
  *x = _x;
  return line;
}

void print_error(const char* name, const char* content, const char* pos,
                 char* fmt, ...) {
  assert(name != NULL);
  assert(content != NULL);
  assert(pos != NULL);
  int x, y;
  const char* line = position_info(content, pos, &x, &y);

  printf("%s:%d:%d: error: ", name, y + 1, x + 1);
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
  
  print_pos(line, x, y);
}

void print_warning(const char* name, const char* content, const char* pos,
                 char* fmt, ...) {
  assert(name != NULL);
  assert(content != NULL);
  assert(pos != NULL);

  int x, y;
  const char* line = position_info(content, pos, &x, &y);

  printf("%s:%d:%d: warning: ", name, y + 1, x + 1);
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
  
  print_pos(line, x, y);
}
