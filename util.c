#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static char* mstrncpy(const char* src, int n) {
  char* a = malloc(n + 1);
  strncpy(a, src, n);
  a[n] = '\0';
  return a;
}

Vec* Vec_new() {
  Vec* self = malloc(sizeof(Vec));
  self->buf_l = 8;
  self->buf = malloc(sizeof(void*) * self->buf_l);
  self->len = 0;
  return self;
}
void Vec_push(Vec* self, void* item) {
  if (self->buf_l == self->len) {
    void** dst = malloc(sizeof(void*) * self->buf_l * 2);
    memcpy(dst, self->buf, sizeof(void*) * self->buf_l);

    void** tmp = self->buf;
    self->buf = dst;
    self->buf_l *= 2;
    free(tmp);
  }
  self->buf[self->len] = item;
  self->len += 1;
}
void* Vec_pop(Vec* self) {
  if (!Vec_empty(self)) {
    void* tmp = self->buf[self->len - 1];
    self->len--;
    return tmp;
  }
  return NULL;
}
void* Vec_get(const Vec* self, uint idx) {
  assert(self->len > idx);
  return self->buf[idx];
}
bool Vec_empty(const Vec* self) { return self->len == 0; }
void Vec_free(Vec* self) {
  free(self->buf);
  free(self);
}

typedef struct MapNode {
  char* key;
  int n;
  void* item;

  struct MapNode* lhs;
  struct MapNode* rhs;
} MapNode;
static MapNode* MapNode_new() {
  MapNode* self = malloc(sizeof(MapNode));
  self->lhs = self->rhs = self->item = NULL;
}
void MapNode_free(MapNode* self) {
  if (self->lhs != NULL) MapNode_free(self->lhs);
  if (self->rhs != NULL) MapNode_free(self->rhs);

  free(self->key);
  free(self);
}
bool MapNode_push(MapNode* self, const char* key, int n, void* item) {
  int cmp = strncmp(self->key, key, n);
  if (cmp > 0 || (cmp == 0 && self->n > n)) {
    if (self->lhs == NULL) {
      MapNode* node = MapNode_new();
      node->item = item;
      node->key = mstrncpy(key, n);
      node->n = n;

      self->lhs = node;
      return true;
    } else {
      return MapNode_push(self->lhs, key, n, item);
    }
  } else if (cmp < 0 || (cmp == 0 && self->n < n)) {
    if (self->lhs == NULL) {
      MapNode* node = MapNode_new();
      node->item = item;
      node->key = mstrncpy(key, n);
      node->n = n;

      self->rhs = node;
      return true;
    } else {
      return MapNode_push(self->rhs, key, n, item);
    }
  } else
    return false;
}
void* MapNode_pushf(MapNode* self, const char* key, int n, void* item) {
  int cmp = strncmp(self->key, key, n);
  if (cmp > 0 || (cmp == 0 && self->n > n)) {
    if (self->lhs == NULL) {
      MapNode* node = MapNode_new();
      node->item = item;
      node->key = mstrncpy(key, n);
      node->n = n;

      self->lhs = node;
      return NULL;
    } else {
      return MapNode_pushf(self->lhs, key, n, item);
    }
  } else if (cmp < 0 || (cmp == 0 && self->n < n)) {
    if (self->lhs == NULL) {
      MapNode* node = MapNode_new();
      node->item = item;
      node->key = mstrncpy(key, n);
      node->n = n;

      self->rhs = node;
      return NULL;
    } else {
      return MapNode_pushf(self->rhs, key, n, item);
    }
  } else {
    // match self
    void* tmp = self->item;
    self->item = item;
    return tmp;
  }
}
void* MapNode_pop(MapNode* self, const char* key, int n) {
  int cmp = strncmp(self->key, key, n);
  if (cmp > 0 || (cmp == 0 && self->n > n)) {
    if (self->lhs == NULL) {
      return NULL;
    } else {
      void* res = MapNode_pop(self->lhs, key, n);
      if (res == self->lhs->item) {
        MapNode_free(self->lhs);
        self->lhs = NULL;
      }
      return res;
    }
  } else if (cmp < 0 || (cmp == 0 && self->n < n)) {
    if (self->lhs == NULL) {
      return NULL;
    } else {
      void* res = MapNode_pop(self->rhs, key, n);
      if (res == self->rhs->item) {
        MapNode_free(self->rhs);
        self->rhs = NULL;
      }
      return res;
    }
  } else {
    void* item = self->item;
    return item;
  }
}
void* MapNode_get(MapNode* self, const char* key, int n) {
  int cmp = strncmp(self->key, key, n);
  if (cmp > 0 || (cmp == 0 && self->n > n)) {
    if (self->lhs == NULL) {
      return NULL;
    } else {
      return MapNode_get(self->lhs, key, n);
    }
  } else if (cmp < 0 || (cmp == 0 && self->n < n)) {
    if (self->lhs == NULL) {
      return NULL;
    } else {
      return MapNode_pop(self->rhs, key, n);
    }
  } else {
    return self->item;
  }
}

Map* Map_new() {
  Map* self = malloc(sizeof(Map));
  self->root = NULL;
  self->len = 0;
  return self;
}
bool Map_push(Map* self, const char* key, int n, void* item) {
  if (Map_empty(self)) {
    MapNode* node = MapNode_new();
    node->key = mstrncpy(key, n);
    node->n = n;
    node->item = item;

    self->root = node;
    self->len++;
    return true;
  } else {
    bool is_pushed = MapNode_push(self->root, key, n, item);
    if (is_pushed) {
      self->len++;
    }
    return is_pushed;
  }
}
void* Map_pushf(Map* self, const char* key, int n, void* item) {
  /**
   * @brief push force
   * @return (exist) ? prev item : NULL
   */
  if (Map_empty(self)) {
    MapNode* node = MapNode_new();
    node->key = mstrncpy(key, n);
    node->n = n;
    node->item = item;

    self->root = node;
    self->len++;
    return NULL;
  } else {
    void *tmp = MapNode_pushf(self->root, key, n, item);
    if(tmp == NULL) self->len++;
    return tmp;
  }
}
void* Map_pop(Map* self, const char* key, int n) {
  if (Map_empty(self)) return NULL;
  return MapNode_pop(self->root, key, n);
}
void* Map_get(Map* self, const char* key, int n) {
  if (Map_empty(self)) return NULL;
  return MapNode_get(self->root, key, n);
}
bool Map_empty(Map* self) { return self->root == NULL; }
void Map_free(Map* self) {
  MapNode_free(self->root);
  free(self);
}
