#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "hydra.h"

#define MAX_NODES 1000000000L
static int64_t nbc,nbd,step,max_nodes;

void print_node(node *n) {
  printf("nb=%4lu adr=%14p dist=%4d parent=%14p next=%14p first=%14p shortest=%14p\n",
	 n->nb,n,n->dist,n->parent,n->next,n->first_child,n->shortest);
  fflush(stdout);
}

void print_tree(node *n) {
  while (n!=NULL) {
    print_node(n);
    print_tree(n->first_child);
    n=n->next;
  }
}

void tree_to_forest(node *n,char *s) {
  char tmp[10];
  while (n!=NULL) {
    sprintf(tmp,"[%ld",n->nb);
    strcat(s,tmp);
    tree_to_forest(n->first_child,s);
    strcat(s,"]");
    n=n->next;
  }
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

void free_tree(node *n) {
  node *curr=n->first_child;
  while (curr!=NULL) {
    node *next=curr->next;
    free_tree(curr);
    curr=next;
  }
  free(n);
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

void del_node(node *n) {
  node *p = n->parent;
  if (n->prev==NULL) p->first_child = n->next;
  else n->prev->next=n->next;
  if (n->next!=NULL) n->next->prev=n->prev;
  update_del(n,false);
  free(n);
  nbd++;
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

node *add_node(node *n) {
  if ((nbc-nbd)>MAX_NODES) {
#ifdef DEBUG
    printf("More than %ld nodes\n",MAX_NODES);
#endif
    return NULL;
  }
  node *new_node = (node *)malloc(sizeof(node));
  new_node->nb=++nbc;
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

node *copy_subtree(node *src,node *dest) {
  while (src!=NULL) {
    if (add_node(dest)==NULL) return NULL;
    if (copy_subtree(src->first_child,dest->first_child)==NULL) return NULL;
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

bool cut_node(node *root, int64_t s) {
  node *n=find_best_leaf(root);
  //pas de parent => on est à la racine, c'est terminé
  if (n->parent==NULL) return true;
  node *parent=n->parent;
  del_node(n);
  node *gp=parent->parent;
  // Si le grand père n'existe pas, il n' y a rien à copier
  if (gp==NULL)  return false;
  // Sinon il faut copier s fois le sous arbre parent dans le grand parent
  for (int64_t i=0;i<s;i++) {
    // Echec de add_node (too many nodes). On renvoie true
    if (add_node(gp)==NULL) return true;
    if (copy_subtree(parent->first_child,gp->first_child)==NULL) return true;
  }
  return false;
}

node *build_tree(char *orig) {
  node *root;
  nbc=1;nbd=0;
  step=1;
  max_nodes=0;
  root=(node *)malloc(sizeof(node));
  root->parent=NULL;
  root->first_child=NULL;
  root->prev=NULL;
  root->next=NULL;
  root->dist=0;
  root->shortest=NULL;
  root->nb=nbc;
  char *s = malloc(strlen(orig)+1);
  strcpy(s,orig);
  while (true) {
    char *ns = strsep(&s,",");
    if (ns==NULL) break;
    int i = atoi(ns);
    node *n=find_node(root,i);
    if (n==NULL) {
      printf("Node %d not  found\n",i);
      exit(0);
    }
    add_node(n);
  }
  free(s);
  return root;
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

char *encode(node *n) {
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
    children_encodings[i] = encode(curr);
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

void hydra(node *root,res_hydra *res) {
  while (true)  {
    if (cut_node(root,step)) {
      res->nb_created=nbc;
      res->nb_deleted=nbd;
      res->max_nodes=max_nodes;
      res->step=step;
      if ((nbc-nbd)>MAX_NODES) res->success=false;
      else res->success=true;
      return;
    }
    if ((nbc-nbd)>max_nodes) max_nodes=nbc-nbd;
    step++;
  }
}
