#include <assert.h>
#include <stdlib.h>

#include "mycc.h"

int test_Vec() {
  int a[256];
  for (int i = 0; i < 256; i++) a[i] = i;
  Vec* vec = Vec_new();

  assert(Vec_empty(vec) == true);

  for (int i = 0; i < 256; i++) {
    Vec_push(vec, &a[i]);
  }

  assert(Vec_empty(vec) == false);
  assert(*(int*)Vec_get(vec, 0) == 0);
  assert(*(int*)Vec_get(vec, 2) == 2);
  assert(*(int*)Vec_get(vec, 17) == 17);

  assert(*(int*)Vec_pop(vec) == 255);
  assert(*(int*)Vec_pop(vec) == 254);

  // assert(Vec_get(vec, 255) == NULL); // abort

  Vec_free(vec);
  return 0;
}

int test_Map() {
  int a[5];
  for(int i=0; i<5; i++) a[i] = i;

  Map* map = Map_new();

  assert(Map_empty(map) == true);

  Map_push(map, "a0", 2, a + 0);
  Map_push(map, "a1", 2, a + 1);
  Map_push(map, "a10", 3, a + 2);
  Map_push(map, "a100", 4, a + 4);
  Map_push(map, "a11", 3, a + 3);

  assert(Map_empty(map) == false);

  assert(*(int*)Map_get(map, "a0", 2) == 0);
  assert(*(int*)Map_get(map, "a10", 3) == 2);
  assert(*(int*)Map_get(map, "a1", 2) == 1);

  assert(*(int*)Map_get(map, "a11001010", 2) == 1);

  assert(*(int*)Map_pop(map, "a1", 2) == 1);
  assert(Map_get(map, "a1", 2) == NULL);

  Map_free(map);
  return 0;
}

int main() {
  return test_Vec() && test_Map();
}