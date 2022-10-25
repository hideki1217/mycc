// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)AvlTree.html

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "collection.h"

/* implementation of an AVL tree with explicit heights */

struct avlNode {
  struct avlNode *child[2]; /* left and right */
  int key;
  int height;
  void *item;
};

/* free a tree */
void Avl_free(AvlTree t) {
  if (t != AVL_EMPTY) {
    Avl_free(t->child[0]);
    Avl_free(t->child[1]);
    free(t);
  }
}

/* return height of an AVL tree */
int Avl_height(AvlTree t) {
  if (t != AVL_EMPTY) {
    return t->height;
  } else {
    return 0;
  }
}

/* return nonzero if key is present in tree */
void *Avl_contain(AvlTree t, int key) {
  if (t == AVL_EMPTY) {
    return NULL;
  } else if (t->key == key) {
    return t->item;
  } else {
    return Avl_contain(t->child[key > t->key], key);
  }
}

#define Max(x, y) ((x) > (y) ? (x) : (y))

/* assert height fields are correct throughout tree */
void Avl_sanitycheck(AvlTree root) {
  int i;

  if (root != AVL_EMPTY) {
    for (i = 0; i < 2; i++) {
      Avl_sanitycheck(root->child[i]);
    }

    assert(root->height ==
           1 + Max(Avl_height(root->child[0]), Avl_height(root->child[1])));
  }
}

/* recompute height of a node */
static void Avl_fix_height(AvlTree t) {
  assert(t != AVL_EMPTY);

  t->height = 1 + Max(Avl_height(t->child[0]), Avl_height(t->child[1]));
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
static void Avl_rotate(AvlTree *root, int d) {
  AvlTree oldRoot;
  AvlTree newRoot;
  AvlTree oldMiddle;

  oldRoot = *root;
  newRoot = oldRoot->child[d];
  oldMiddle = newRoot->child[!d];

  oldRoot->child[d] = oldMiddle;
  newRoot->child[!d] = oldRoot;
  *root = newRoot;

  /* update heights */
  Avl_fix_height((*root)->child[!d]); /* old root */
  Avl_fix_height(*root);              /* new root */
}

/* rebalance at node if necessary */
/* also fixes height */
static void Avl_rebalance(AvlTree *t) {
  int d;

  if (*t != AVL_EMPTY) {
    for (d = 0; d < 2; d++) {
      /* maybe child[d] is now too tall */
      if (Avl_height((*t)->child[d]) > Avl_height((*t)->child[!d]) + 1) {
        /* imbalanced! */
        /* how to fix it? */
        /* need to look for taller grandchild of child[d] */
        if (Avl_height((*t)->child[d]->child[d]) >
            Avl_height((*t)->child[d]->child[!d])) {
          /* same direction grandchild wins, do single rotation */
          Avl_rotate(t, d);
        } else {
          /* opposite direction grandchild moves up, do double rotation */
          Avl_rotate(&(*t)->child[d], !d);
          Avl_rotate(t, d);
        }

        return; /* Avl_rotate called Avl_fix_height */
      }
    }

    /* update height */
    Avl_fix_height(*t);
  }
}

/* insert into tree */
/* this may replace root, which is why we pass
 * in a AvlTree * */
int Avl_push(AvlTree *t, int key, void *item) {
  /* insertion procedure */
  if (*t == AVL_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct avlNode));
    assert(*t);

    (*t)->child[0] = AVL_EMPTY;
    (*t)->child[1] = AVL_EMPTY;

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
    int res = Avl_push(&(*t)->child[key > (*t)->key], key, item);

    Avl_rebalance(t);

    return res;
  }
}
void *Avl_pushf(AvlTree *t, int key, void *item) {
  /* insertion procedure */
  if (*t == AVL_EMPTY) {
    /* new t */
    *t = malloc(sizeof(struct avlNode));
    assert(*t);

    (*t)->child[0] = AVL_EMPTY;
    (*t)->child[1] = AVL_EMPTY;

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
    void *res = Avl_pushf(&(*t)->child[key > (*t)->key], key, item);
    Avl_rebalance(t);
    return res;
  }
}

/* print all elements of the tree in order */
void Avl_print_keys(AvlTree t) {
  if (t != AVL_EMPTY) {
    Avl_print_keys(t->child[0]);
    printf("%d\n", t->key);
    Avl_print_keys(t->child[1]);
  }
}

/* delete and return minimum value in a tree */
static int Avl_delete_min(AvlTree *t, void **item) {
  AvlTree oldroot;
  int minValue;

  assert(t != AVL_EMPTY);

  if ((*t)->child[0] == AVL_EMPTY) {
    /* root is min value */
    oldroot = *t;
    minValue = oldroot->key;
    *item = oldroot->item;
    *t = oldroot->child[1];
    free(oldroot);
  } else {
    /* min value is in left subtree */
    minValue = Avl_delete_min(&(*t)->child[0], item);
  }

  Avl_rebalance(t);
  return minValue;
}

/* delete the given value */
void *Avl_delete(AvlTree *t, int key) {
  AvlTree oldroot;

  if (*t != AVL_EMPTY) {
    return NULL;
  } else if ((*t)->key == key) {
    void *item = (*t)->item;
    /* do we have a right child? */
    if ((*t)->child[1] != AVL_EMPTY) {
      /* give root min value in right subtree */
      (*t)->key = Avl_delete_min(&(*t)->child[1], &(*t)->item);
    } else {
      /* splice out root */
      oldroot = (*t);
      *t = (*t)->child[0];
      free(oldroot);
    }
    return item;
  } else {
    return Avl_delete(&(*t)->child[key > (*t)->key], key);
  }

  /* rebalance */
  Avl_rebalance(t);
}