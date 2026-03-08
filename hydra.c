#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "hydra.h"

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

struct _state_hydra {
  node *root;
  int64_t nodes_limit;
  int64_t nb_created; // number of nodes created
  int64_t nb_deleted; // number of nodes deleted
  int64_t max_nodes;  // maximal number of nodes reached
  int64_t step;       // number of steps
  hydra_status status;       
};

int64_t get_nodes_limit(state_hydra *s) {
  return s->nodes_limit;
}
int64_t get_nb_created(state_hydra *s) {
  return s->nb_created;
}
int64_t get_nb_deleted(state_hydra *s) {
  return s->nb_deleted;
}
int64_t get_max_nodes(state_hydra *s) {
  return s->max_nodes;
}
int64_t get_steps(state_hydra *s) {
  return s->step;
}
hydra_status get_status(state_hydra *s) {
  return s->status;
}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })



void print_node(node *n) {
  printf("nb=%4lu adr=%14p dist=%4d parent=%14p next=%14p first=%14p shortest=%14p\n",
	 n->nb,n,n->dist,n->parent,n->next,n->first_child,n->shortest);
  fflush(stdout);
}

void print_nodes(node *n) {
  while (n!=NULL) {
    print_node(n);
    print_nodes(n->first_child);
    n=n->next;
  }
}

void print_hydra(state_hydra *s) {
  printf("nodes_limit=%ld nb_created=%ld nb_deleted=%ld max_nodes=%ld steps=%ld\n",
	 s->nodes_limit,s->nb_created,s->nb_deleted,s->max_nodes,s->step);
  print_nodes(s->root);
}

void nodes_to_forest(node *n,char *s) {
  char tmp[10];
  while (n!=NULL) {
    sprintf(tmp,"[%ld",n->nb);
    strcat(s,tmp);
    nodes_to_forest(n->first_child,s);
    strcat(s,"]");
    n=n->next;
  }
}

void hydra_to_forest(state_hydra *state, char *s) {
  nodes_to_forest(state->root,s);
}

node *find_node(node *n,int64_t nb) {
  while (n!=NULL) {
    if (n->nb==nb) return n;
    node *ret=find_node(n->first_child,nb);
    if (ret!=NULL) return ret;
    n=n->next;
  }
  return NULL;
}

void free_nodes(node *n) {
  node *curr=n->first_child;
  while (curr!=NULL) {
    node *next=curr->next;
    free_nodes(curr);
    curr=next;
  }
  free(n);
}

void free_hydra(state_hydra *s) {
  free_nodes(s->root);
  free(s);
}

void update_del(node *n,bool alive) {
  node *p = n->parent;
  if (p==NULL) return;
  uint16_t old_d=p->dist;
  if (p->first_child==NULL) {
    p-> dist=0;
    p-> shortest=NULL;
  }
  else {
    if (p->shortest!=n) {
      // Si le noeud n'était pas le plus court est qu'il ne permettra pas de raccourcir la distance, c'est terminé
      if ((n->dist+1)>=p->dist) return;
      // Sinon c'est le nouveau shortest
      else {
	p->dist=n->dist+1;
	p->shortest=n;
      }
    }
    else {
      // Le noeud était le plus court et il ne vient pas d'être détruit
      if (alive && ((n->dist+1) <=p->dist)) {
	// Il reste le plus court, puisque la nouvelle valeur est inférieure ou égale à l'ancienne
	p->dist=n->dist+1;
      }
      else {
	// Il faut tout vérifier
	node *curr=p->first_child;
	p->dist=32767;
	while (curr!=NULL) {
	  if ((curr->dist+1) < p->dist) {
	    p->dist = curr->dist+1;
	    p->shortest=curr;
	    if (p->dist==1) break;
	  }
	  curr=curr->next;
	}
      }
    }
  }
  // Si la valeur a changé, il faut updater les noeuds supérieurs
  if (old_d!=p->dist) update_del(p,true);
}

void del_node(node *n,state_hydra *state) {
  node *p = n->parent;
  if (n->prev==NULL) p->first_child = n->next;
  else n->prev->next=n->next;
  if (n->next!=NULL) n->next->prev=n->prev;
  update_del(n,false);
  free(n);
  state->nb_deleted++;
}

void update_add(node *n) {
  node *p=n->parent;
  // Si le noeud n'était pas le plus court, aucun update à faire
  if ((p==NULL)||(p->shortest!=n)) return;
  node *curr = p->first_child;
  uint16_t old_d = p->dist;
  p->dist=n->dist+1;
  while (curr!=NULL) {
    if (curr->dist<n->dist) {
      p->shortest=curr;
      p->dist=curr->dist+1;
      // La distance ne peut croitre que de 1, donc on arrête dès que l'on en trouve un inférieur au nouveau
      break;
    }
    curr=curr->next;
  }
  // Si la distance n'a pas été modifiée => pas d'update à faire
  if (p->dist!=old_d) update_add(p);
}

node *add_node(node *n,state_hydra *state) {
  if ((state->nb_created-state->nb_deleted)>state->nodes_limit) {
#ifdef DEBUG
    printf("More than %ld nodes\n",state->nodes_limit);
#endif
    return NULL;
  }
  node *new_node = (node *)malloc(sizeof(node));
  new_node->nb=++state->nb_created;
  new_node->prev=NULL;
  new_node->next=n->first_child;
  new_node->parent=n;
  new_node->first_child = NULL;
  new_node->dist=0;
  new_node->shortest=NULL;
  n->first_child=new_node;
  n->dist=1;
  n->shortest=new_node;
  update_add(n);
  return new_node;
}

node *copy_subtree(node *src,node *dest,state_hydra *state) {
  while (src!=NULL) {
    if (add_node(dest,state)==NULL) return NULL;
    if (copy_subtree(src->first_child,dest->first_child,state)==NULL) return NULL;
    src=src->next;
  }
  return dest;
}

node *find_best_leaf(node *n) {
  while (n->shortest!=NULL) {
    n=n->shortest;
  }
  return n;
}

bool cut_node(state_hydra *state) {
  node *n=find_best_leaf(state->root);
  //pas de parent => on est à la racine, c'est terminé
  if (n->parent==NULL) return true;
  node *parent=n->parent;
  del_node(n,state);
  node *gp=parent->parent;
  // Si le grand père n'existe pas, il n' y a rien à copier
  if (gp==NULL)  return false;
  // Sinon il faut copier s fois le sous arbre parent dans le grand parent
  for (int64_t i=0;i<state->step;i++) {
    // Echec de add_node (too many nodes). On renvoie true
    if (add_node(gp,state)==NULL) return true;
    if (copy_subtree(parent->first_child,gp->first_child,state)==NULL) return true;
  }
  return false;
}

state_hydra *build_hydra(char *orig,int64_t max_search) {
  state_hydra *state=(state_hydra *)malloc(sizeof(state_hydra));
  struct sysinfo sinf;
  state->nb_created=1;
  state->nb_deleted=0;
  state->step=1;
  state->max_nodes=0;
  state->status=RUNNING;
  sysinfo(&sinf);
  int64_t max_mem = (int64_t)sinf.mem_unit* (int64_t)sinf.totalram;
  state->nodes_limit = (max_mem-1000000000L)/sizeof(node);
  if (max_search!=0) state->nodes_limit=min(max_search,state->nodes_limit);
  state->root=(node *)malloc(sizeof(node));
  state->root->parent=NULL;
  state->root->first_child=NULL;
  state->root->prev=NULL;
  state->root->next=NULL;
  state->root->dist=0;
  state->root->shortest=NULL;
  state->root->nb=state->nb_created;
  char *s = malloc(strlen(orig)+1);
  strcpy(s,orig);
  while (true) {
    char *ns = strsep(&s,",");
    if (ns==NULL) break;
    int i = atoi(ns);
    node *n=find_node(state->root,i);
    if (n==NULL) {
      printf("Node %d not  found\n",i);
      exit(0);
    }
    add_node(n,state);
  }
  free(s);
  return state;
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

char *encode_nodes(node *n) {
  int num_children=0;
  node *curr=n->first_child;
  while (curr!=NULL) {
    num_children++;
    curr=curr->next;
  }
  if (num_children==0) {
    char* leaf_str = (char*)malloc(3 * sizeof(char)); 
    strcpy(leaf_str, "()");
    return leaf_str;
  }
  char** children_encodings = (char**)malloc(num_children * sizeof(char*));
  int total_length = 0;
  curr=n->first_child;
  for (int i = 0; i < num_children; i++) {
    children_encodings[i] = encode_nodes(curr);
    curr=curr->next;
    total_length += strlen(children_encodings[i]);
  }
  qsort(children_encodings, num_children, sizeof(char*), compare_strings);
  char* parent_str = (char*)malloc((total_length + 3) * sizeof(char));
  strcpy(parent_str, "(");
  for (int i = 0; i < num_children; i++) {
    strcat(parent_str, children_encodings[i]);
    free(children_encodings[i]); 
  }
  strcat(parent_str, ")");
  free(children_encodings); 
  return parent_str;
}

char *encode_hydra(state_hydra *state) {
  return encode_nodes(state->root);
}

bool one_step(state_hydra *state) {
  bool result = cut_node(state);
  if ((state->nb_created-state->nb_deleted)>state->max_nodes) state->max_nodes=state->nb_created-state->nb_deleted;
  if (result) {
    if ((state->nb_created-state->nb_deleted)>state->nodes_limit) state->status=FAILURE;
    else state->status=SUCCESS;
    return true;
  }
  state->step++;
  return false;
}

void hydra(state_hydra *state) {
  while (true)  {
    if (cut_node(state)) {
      if ((state->nb_created-state->nb_deleted)>state->nodes_limit) state->status=FAILURE;
      else state->status=SUCCESS;;
      return;
    }
    if ((state->nb_created-state->nb_deleted)>state->max_nodes) state->max_nodes=state->nb_created-state->nb_deleted;
    state->step++;
  }
}
