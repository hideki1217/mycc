#include <stdlib.h>

#include "mycc.h"

struct setNode {
  long item;
  struct setNode* next;
};

static Set node_new(long item) {
  Set set = malloc(sizeof(struct setNode));
  set->item = item;
  set->next = SET_EMPTY;
  return set;
}

bool Set_push(Set self, long item) {
  if (self->item == item) return false;
  if (self->next == SET_EMPTY) {
    self->next = node_new(item);
    return true;
  } else
    return Set_push(self->next, item);
}
Set Set_cpy(Set self) {
  Set res = Set_new();
  for (; self != SET_EMPTY; self = self->next) Set_push(res, self->item);
  return res;
}
Set Set_union(Set self, Set rhs) {
  Set res = Set_new();
  for (Set r = rhs; r != SET_EMPTY; r = r->next) Set_push(res, r->item);
  for (Set l = self; l != SET_EMPTY; l = l->next) Set_push(res, l->item);
  return res;
}
Set Set_cross(Set self, Set rhs) {
  Set res = Set_new();
  for (Set r = rhs; r != SET_EMPTY; r = r->next) {
    for (Set l = self; l != SET_EMPTY; l = l->next) {
      if (r->item == l->item) {
        Set_push(res, r->item);
        break;
      }
    }
  }
  return res;
}
void Set_free(Set self) {
  if (self == SET_EMPTY) return;
  Set_free(self->next);
  free(self);
}
