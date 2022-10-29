// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Dict.html

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "mycc.h"

/* implementation of an AVL tree with explicit heights */

typedef struct DictNode *Dict;
#define DICT_EMPTY NULL

struct DictNode {
  struct DictNode *child[2]; /* left and right */
  const char *key;
  int height;
};

#define key_cmp(lhs, rhs) (strcmp((lhs), (rhs)))

/* free a tree */
void Dict_free(Dict t) {
  if (t != DICT_EMPTY) {
    Dict_free(t->child[0]);
    Dict_free(t->child[1]);
    free(t);
  }
}

static int Dict_height(Dict t) {
  if (t != DICT_EMPTY) {
    return t->height;
  } else {
    return 0;
  }
}

const char *Dict_contain(Dict t, const char *key) {
  if (t == DICT_EMPTY) {
    return NULL;
  }

  int cmp = key_cmp(key, t->key);
  if (cmp == 0) {
    return t->key;
  } else {
    return Dict_contain(t->child[cmp < 0 ? 0 : 1], key);
  }
}

#define Max(x, y) ((x) > (y) ? (x) : (y))

/* assert height fields are correct throughout tree */
static void Dict_sanitycheck(Dict root) {
  int i;

  if (root != DICT_EMPTY) {
    for (i = 0; i < 2; i++) {
      Dict_sanitycheck(root->child[i]);
    }

    assert(root->height ==
           1 + Max(Dict_height(root->child[0]), Dict_height(root->child[1])));
  }
}

static void Dict_fix_height(Dict t) {
  assert(t != DICT_EMPTY);

  t->height = 1 + Max(Dict_height(t->child[0]), Dict_height(t->child[1]));
}

/* rotate child[d] to root */
/* assumes child[d] exists */
/* Picture:
 *
 *     y            x
 *    / \   <==>   / \
 *   x   C        A   y
 *  / \              / \
 * A   B            B   C
 *
 */
static void Dict_rotate(Dict *root, int d) {
  Dict oldRoot;
  Dict newRoot;
  Dict oldMiddle;

  oldRoot = *root;
  newRoot = oldRoot->child[d];
  oldMiddle = newRoot->child[!d];

  oldRoot->child[d] = oldMiddle;
  newRoot->child[!d] = oldRoot;
  *root = newRoot;

  /* update heights */
  Dict_fix_height((*root)->child[!d]); /* old root */
  Dict_fix_height(*root);              /* new root */
}

/* rebalance at node if necessary */
/* also fixes height */
static void Dict_rebalance(Dict *t) {
  int d;

  if (*t != DICT_EMPTY) {
    for (d = 0; d < 2; d++) {
      /* maybe child[d] is now too tall */
      if (Dict_height((*t)->child[d]) > Dict_height((*t)->child[!d]) + 1) {
        /* imbalanced! */
        /* how to fix it? */
        /* need to look for taller grandchild of child[d] */
        if (Dict_height((*t)->child[d]->child[d]) >
            Dict_height((*t)->child[d]->child[!d])) {
          /* same direction grandchild wins, do single rotation */
          Dict_rotate(t, d);
        } else {
          /* opposite direction grandchild moves up, do double rotation */
          Dict_rotate(&(*t)->child[d], !d);
          Dict_rotate(t, d);
        }

        return; /* Dict_rotate called Dict_fix_height */
      }
    }

    /* update height */
    Dict_fix_height(*t);
  }
}

/* insert into tree */
/* this may replace root, which is why we pass
 * in a Dict * */
const char *Dict_push(Dict *t, const char *key) {
  /* insertion procedure */
  if (*t == DICT_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct DictNode));
    assert(*t);

    (*t)->child[0] = DICT_EMPTY;
    (*t)->child[1] = DICT_EMPTY;

    (*t)->key = key;

    (*t)->height = 1;

    /* done */
    return key;
  }

  int cmp = key_cmp(key, (*t)->key);
  if (cmp == 0) {
    /* nothing to do */
    return (*t)->key;
  } else {
    /* do the insert in subtree */
    const char *res = Dict_push(&(*t)->child[cmp < 0 ? 0 : 1], key);

    Dict_rebalance(t);

    return res;
  }
}

const char *Dict_push_copy(Dict *t, const char *key) {
  /* insertion procedure */
  if (*t == DICT_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct DictNode));
    assert(*t);

    (*t)->child[0] = DICT_EMPTY;
    (*t)->child[1] = DICT_EMPTY;

    char* cpy = malloc(strlen(key) + 1);
    strcpy(cpy, key);
    (*t)->key = cpy;

    (*t)->height = 1;

    /* done */
    return cpy;
  }

  int cmp = key_cmp(key, (*t)->key);
  if (cmp == 0) {
    /* nothing to do */
    return (*t)->key;
  } else {
    /* do the insert in subtree */
    const char *res = Dict_push_copy(&(*t)->child[cmp < 0 ? 0 : 1], key);

    Dict_rebalance(t);

    return res;
  }
}

/* print all elements of the tree in order */
void Dict_print_keys(Dict t) {
  if (t != DICT_EMPTY) {
    Dict_print_keys(t->child[0]);
    printf("%s\n", t->key);
    Dict_print_keys(t->child[1]);
  }
}
