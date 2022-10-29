// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Map.html

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mycc.h"

/* implementation of an AVL tree with explicit heights */

struct mapNode {
  struct mapNode *child[2]; /* left and right */
  long key;
  int height;
  void *item;
};

/* free a tree */
void Map_free(Map t) {
  if (t != MAP_EMPTY) {
    Map_free(t->child[0]);
    Map_free(t->child[1]);
    free(t);
  }
}

/* return height of an AVL tree */
static int Map_height(Map t) {
  if (t != MAP_EMPTY) {
    return t->height;
  } else {
    return 0;
  }
}

/* return nonzero if key is present in tree */
void *Map_contain(Map t, long key) {
  if (t == MAP_EMPTY) {
    return NULL;
  } else if (t->key == key) {
    return t->item;
  } else {
    return Map_contain(t->child[key > t->key], key);
  }
}

#define Max(x, y) ((x) > (y) ? (x) : (y))

/* assert height fields are correct throughout tree */
static void Map_sanitycheck(Map root) {
  int i;

  if (root != MAP_EMPTY) {
    for (i = 0; i < 2; i++) {
      Map_sanitycheck(root->child[i]);
    }

    assert(root->height ==
           1 + Max(Map_height(root->child[0]), Map_height(root->child[1])));
  }
}

/* recompute height of a node */
static void Map_fix_height(Map t) {
  assert(t != MAP_EMPTY);

  t->height = 1 + Max(Map_height(t->child[0]), Map_height(t->child[1]));
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
static void Map_rotate(Map *root, int d) {
  Map oldRoot;
  Map newRoot;
  Map oldMiddle;

  oldRoot = *root;
  newRoot = oldRoot->child[d];
  oldMiddle = newRoot->child[!d];

  oldRoot->child[d] = oldMiddle;
  newRoot->child[!d] = oldRoot;
  *root = newRoot;

  /* update heights */
  Map_fix_height((*root)->child[!d]); /* old root */
  Map_fix_height(*root);              /* new root */
}

/* rebalance at node if necessary */
/* also fixes height */
static void Map_rebalance(Map *t) {
  int d;

  if (*t != MAP_EMPTY) {
    for (d = 0; d < 2; d++) {
      /* maybe child[d] is now too tall */
      if (Map_height((*t)->child[d]) > Map_height((*t)->child[!d]) + 1) {
        /* imbalanced! */
        /* how to fix it? */
        /* need to look for taller grandchild of child[d] */
        if (Map_height((*t)->child[d]->child[d]) >
            Map_height((*t)->child[d]->child[!d])) {
          /* same direction grandchild wins, do single rotation */
          Map_rotate(t, d);
        } else {
          /* opposite direction grandchild moves up, do double rotation */
          Map_rotate(&(*t)->child[d], !d);
          Map_rotate(t, d);
        }

        return; /* Map_rotate called Map_fix_height */
      }
    }

    /* update height */
    Map_fix_height(*t);
  }
}

/* insert into tree */
/* this may replace root, which is why we pass
 * in a Map * */
int Map_push(Map *t, long key, void *item) {
  /* insertion procedure */
  if (*t == MAP_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct mapNode));
    assert(*t);

    (*t)->child[0] = MAP_EMPTY;
    (*t)->child[1] = MAP_EMPTY;

    (*t)->key = key;
    (*t)->item = item;

    (*t)->height = 1;

    /* done */
    return 1;
  } else if (key == (*t)->key) {
    /* nothing to do */
    return 0;
  } else {
    /* do the insert in subtree */
    int res = Map_push(&(*t)->child[key > (*t)->key], key, item);

    Map_rebalance(t);

    return res;
  }
}
void *Map_pushf(Map *t, long key, void *item) {
  /* insertion procedure */
  if (*t == MAP_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct mapNode));
    assert(*t);

    (*t)->child[0] = MAP_EMPTY;
    (*t)->child[1] = MAP_EMPTY;

    (*t)->key = key;
    (*t)->item = item;

    (*t)->height = 1;

    /* done */
    return NULL;
  } else if (key == (*t)->key) {
    void *item = (*t)->item;
    (*t)->item = item;
    /* nothing to do */
    return item;
  } else {
    /* do the insert in subtree */
    void *res = Map_pushf(&(*t)->child[key > (*t)->key], key, item);
    Map_rebalance(t);
    return res;
  }
}

/* print all elements of the tree in order */
void Map_print_keys(Map t) {
  if (t != MAP_EMPTY) {
    Map_print_keys(t->child[0]);
    printf("%ld\n", t->key);
    Map_print_keys(t->child[1]);
  }
}

/* delete and return minimum value in a tree */
static int Map_delete_min(Map *t, void **item) {
  Map oldroot;
  int minValue;

  assert(t != MAP_EMPTY);

  if ((*t)->child[0] == MAP_EMPTY) {
    /* root is min value */
    oldroot = *t;
    minValue = oldroot->key;
    *item = oldroot->item;
    *t = oldroot->child[1];
    free(oldroot);
  } else {
    /* min value is in left subtree */
    minValue = Map_delete_min(&(*t)->child[0], item);
  }

  Map_rebalance(t);
  return minValue;
}

/* delete the given value */
void *Map_delete(Map *t, long key) {
  Map oldroot;

  if (*t != MAP_EMPTY) {
    return NULL;
  } else if ((*t)->key == key) {
    void *item = (*t)->item;
    /* do we have a right child? */
    if ((*t)->child[1] != MAP_EMPTY) {
      /* give root min value in right subtree */
      (*t)->key = Map_delete_min(&(*t)->child[1], &(*t)->item);
    } else {
      /* splice out root */
      oldroot = (*t);
      *t = (*t)->child[0];
      free(oldroot);
    }
    return item;
  } else {
    return Map_delete(&(*t)->child[key > (*t)->key], key);
  }

  /* rebalance */
  Map_rebalance(t);
}