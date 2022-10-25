// https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)AvlTree.html

typedef struct avlNode *AvlTree;

/* empty avl tree is just a null pointer */

#define AVL_EMPTY (0)

/* free a tree */
void Avl_free(AvlTree t);

/* return the height of a tree */
int Avl_height(AvlTree t);

/* return nonzero if key is present in tree */
void* Avl_contain(AvlTree t, int key);

/* insert a new element into a tree */
/* note *t is actual tree */
int Avl_push(AvlTree *t, int key, void* item);
void *Avl_pushf(AvlTree *t, int key, void *item);

/* run sanity checks on tree (for debugging) */
/* assert will fail if heights are wrong */
void Avl_sanitycheck(AvlTree t);

/* print all keys of the tree in order */
void Avl_print_keys(AvlTree t);
