#include <stdint.h>
#include <stdbool.h>

typedef struct _state_hydra state_hydra;
typedef enum _hydra_status {RUNNING, SUCCESS, FAILURE } hydra_status;

// maximal number of nodes before stopping
int64_t get_nodes_limit(state_hydra *state) ;
// Number of nodes created
int64_t get_nb_created(state_hydra *state) ;
// Number of nodes deleted
int64_t get_nb_deleted(state_hydra *state) ;
// Maximal number of nodes reached
int64_t get_max_nodes(state_hydra *state) ;
// Number of steps executed
int64_t get_steps(state_hydra *state) ;
// Status of the computation
hydra_status get_status(state_hydra *state) ;


// Build a hydra and returns the root
// computation stops with a failure when nodes_limit is reached
// If nodes_limit is 0 then (almost) all available memory will be used
// The format string is very special
// "1,2,3,1" means that we attach a node to node 1 (the root), then a node to node 2 (below the root), then a node to node 3 (below node 2), and last a node to the root again.
// "1,1,1,1,2,3,4,5" means that we create first 4 nodes below the root (nodes 2,3,4,5) and then we attach to each of this node another node
// Nodes are always numbered by the order of their creation
// hydra MUST BE freed after use
state_hydra *build_hydra(char *orig,int64_t nodes_limit) ;

// Free a hydra.
// hydra MUST BE freed after use
void  free_hydra(state_hydra *state);

// Solves the hydra
void hydra(state_hydra *state);

// Makes just one iteration and updates s
// Returns true if computation is over, else returns false
bool one_step(state_hydra *state);

// Returns Aho Hopcroft Ullman encoding for the hydra. Helps for finding isomorphic hydras
// Memory for the string is allocated inside the function, so it is the responsability of the caller to free it
char *encode_hydra(state_hydra *state);

// Prints hydra
void print_hydra(state_hydra *state);

// Returns a string encoding the current tree of the hydra as a forest compatible with the LaTeX package forest
// s must be allocated before the call
void hydra_to_forest(state_hydra *state,char *s);
