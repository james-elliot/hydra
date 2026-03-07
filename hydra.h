#include <stdint.h>
#include <stdbool.h>

typedef struct _node node;

typedef struct _res_hydra res_hydra;
struct _res_hydra {
  int64_t nb_created; // number of nodes created
  int64_t nb_deleted; // number of nodes deleted
  int64_t max_nodes;  // maximal number of nodes reached
  int64_t step;       // number of steps
  bool success;       // true if solved, false if max_nodes reached
};

// Solves the hydra and free all memory allocated
void hydra(node *root,res_hydra *res);

// Makes just one iteration and updates res
// Returns true and free memory if computation is over, else return false
bool one_step(node *root,res_hydra *res);

// Free a tree. 
// Use only when stopping in mid resolution step by step, the hydra function takes care of memory management
void  free_tree(node *root);

// Returns Aho Hopcroft Ullman encoding for the graph. Helps for finding isomorphic graphs
char *encode(node *root);

// Build a tree and returns the root
// The format string is very special
// "1,2,3,1" means that we attach a node to node 1 (the root), then a node to node 2 (below the root), then a node to node 3 (below node 2), and last a node to the root again.
// "1,1,1,1,2,3,4,5" means that we create first 4 nodes below the root (nodes 2,3,4,5) and then we attach to each of this node another node
// Nodes are always numbered by the order of their creation
// If max_nodes is 0 then all memory will be used
node *build_tree(char *orig,int64_t max_nodes) ;

void print_tree(node *n);
void tree_to_forest(node *n,char *s);
