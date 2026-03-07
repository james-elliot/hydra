#include "hydra.h"
#include <stdio.h>
#include <string.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

char *acc[1000];
int max_graph=0;
void generate(int max,int nb,char *s) {
  int len = strlen(s);
  if (nb==0) {
    s[len-1]=0;
    //    printf("%s\n",s);
    node *root=build_tree(s);
    char *enc = encode(root);
    //    printf("%s\n",enc);
    int i;
    for (i=0;i<max_graph;i++)
      if (strcmp(acc[i],enc)==0) {
	//	printf("%s is similar to another graph\n",s);
	break;
      }
    if (i==max_graph) {
      char forest[1000];
      res_hydra res;
      forest[0]=0;
      acc[max_graph++]=enc;
      tree_to_forest(root,forest);
      hydra(root,&res);
      if (res.success)
	printf("%s solved in %ld\n",forest,res.step);
      else
	printf("%s could not be solved in %ld\n",forest,res.step);
    }
    free_tree(root);
    return;
  }
  char si[100];
  for (int i=1;i<=max;i++) {
    sprintf(si,"%d",i);
    strcat(s,si);
    strcat(s,",");
    int nmax = max(i+1,max);
    generate(nmax,nb-1,s);
    s[len]=0;
  }
}

int main() {
  char s[2000];
  s[0]=0;
  generate(1,3,s);
  return 0;
}
