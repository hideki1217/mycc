#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

int test_Vec() {
  int a[256];
  for (int i = 0; i < 256; i++) a[i] = i;
  IntV* int_v = IntV_new();

  assert(IntV_empty(int_v) == true);

  for (int i = 0; i < 256; i++) {
    int* ref = IntV_push(int_v);
    *ref = a[i];
  }

  assert(IntV_empty(int_v) == false);
  assert(*IntV_get(int_v, 0) == 0);
  assert(*IntV_get(int_v, 2) == 2);
  assert(*IntV_get(int_v, 17) == 17);

  IntV_pop(int_v);
  IntV_pop(int_v);

  // assert(Vec_get(vec, 255) == NULL); // abort

  IntV_free(int_v);
  return 0;
}

// int test_Map() {
//   int a[5];
//   for (int i = 0; i < 5; i++) a[i] = i;

//   Map* map = Map_new();

//   assert(Map_empty(map) == true);

//   Map_push(map, "a0", 2, a + 0);
//   Map_push(map, "a1", 2, a + 1);
//   Map_push(map, "a10", 3, a + 2);
//   Map_push(map, "a100", 4, a + 4);
//   Map_push(map, "a11", 3, a + 3);

//   assert(Map_empty(map) == false);
//   assert(Map_push(map, "a0", 2, a + 1) == false);
//   assert(Map_pushf(map, "a100", 4, a + 1) == a + 4);
//   assert(Map_pushf(map, "a100", 4, a + 4) == a + 1);

//   assert(*(int*)Map_get(map, "a0", 2) == 0);
//   assert(*(int*)Map_get(map, "a10", 3) == 2);
//   assert(*(int*)Map_get(map, "a1", 2) == 1);

//   assert(*(int*)Map_get(map, "a11001010", 2) == 1);

//   assert(*(int*)Map_pop(map, "a1", 2) == 1);
//   assert(Map_get(map, "a1", 2) == NULL);

//   Map_free(map);
//   return 0;
// }

int test_Dict() {
  Dict* dict = Dict_new();

  assert(Dict_empty(dict) == true);

  for (int j = 0; j < 10; j++) {
    char* x = malloc(11);
    for (int i = 0; i < 10; i++) x[i] = 'a' + j;
    x[10] = '\0';
    Dict_push(dict, x);
  }

  assert(Dict_empty(dict) == false);

  char x[11];
  
  for (int i = 0; i < 10; i++) x[i] = 'a';
  x[10] = '\0';
  assert(Dict_contain(dict, x) == true);
  
  x[2] = 'b';
  assert(Dict_contain(dict, x) == false);

  x[2] = 'a';
  const char* res = Dict_push(dict, x);
  for(int i=0; i<10; i++) assert(res[i] == 'a');

  Dict_free(dict);

  return 0;
}

int test_path() {
  const char* s = "/abs/./../../.vscode/./devcontainer.json";
  char res[256];
  char name[64];

  assert(pathisabs(s) == true);
  pathnorm(res, s);
  assert(strcmp(res, "/.vscode/devcontainer.json") == 0);
  pathname(name, s);
  assert(strcmp(name, "devcontainer.json") == 0);

  pathparen(res, s);
  assert(strcmp(res, "/.vscode") == 0);

  pathcat(res, name);
  assert(strcmp(res, "/.vscode/devcontainer.json") == 0);
  return 0;
}

int main() { return test_Dict() || test_Vec() || test_path(); }