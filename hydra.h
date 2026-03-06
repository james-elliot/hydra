#include <stdint.h>

typedef struct _node node;
struct _node {
  node *parent;
  node *first_child;
  node *prev;
  node *next;
  uint16_t dist;
  int64_t nb;
  node *shortest;
};

// Solves the hydra. Returns the number of steps. If the number is negative, the function had an OUT_OF_MEMORY
int64_t hydra(node *root);

// Returns Aho Hopcroft Ullman encoding for the graph. Helps for finding isomorphic graphs
char *encode(node *root);

// Free a tree
void  free_tree(node *root);

// Build a tree and returns the root
// The format string is very special
// "1,2,3,1" means that we attach a node to node 1 (the root), then a node to node 2 (below the root), then a node to node 3 (below node 2), and last a node to the root again.
// "1,1,1,1,2,3,4,5" means that we create first 4 nodes below the root (nodes 2,3,4,5) and then we attach to each of this node another node
// Nodes are always numbered by the order of their creation
node *build_tree(char *orig) ;
