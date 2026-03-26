#include "hydra.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

char *graphs[1000];
char *encs[1000];
char *forests[1000];
int nb_steps[1000];
int max_graph=0;
void generate(int max,int nb,char *s) {
  int len = strlen(s);
  if (nb==0) {
    s[len-1]=0;
    //    printf("%s\n",s);
    state_hydra *state;
    state=build_hydra(s,1000000);
    char *enc = encode_hydra(state);
    //    printf("%s\n",enc);
    int i;
    for (i=0;i<max_graph;i++)
      if (strcmp(encs[i],enc)==0) {
	//	printf("%s is similar to another graph\n",s);
	free(enc);
	free_hydra(state);
	break;
      }
    if (i==max_graph) {
      char forest[1000];
      hydra_to_forest(state,forest);
      hydra(state);
      hydra_status res=get_status(state);
      if (res==SUCCESS) {
	printf("%s solved in %ld\n",forest,get_steps(state));
        nb_steps[max_graph]=get_steps(state);
      }
      else {
	printf("%s could not be solved in %ld\n",forest,get_steps(state));
        nb_steps[max_graph]=-get_steps(state);
      }
      graphs[max_graph]=(char *)malloc(strlen(s)+1);
      strcpy(graphs[max_graph],s);
      forests[max_graph]=(char *)malloc(strlen(forest)+1);
      strcpy(forests[max_graph],forest);
      encs[max_graph]=enc;
      free_hydra(state);
      max_graph++;
    }
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
  generate(1,4,s);
  int mini=0,ind=-1;
  for (int i=0;i<max_graph;i++) {
    if (nb_steps[i]>mini) {
      mini=nb_steps[i];
      ind=i;
    }
  }
  if (ind!=-1) {
    printf("max_steps= %d\n", nb_steps[ind]);
    printf("%s\n", forests[ind]);
    state_hydra *state=build_hydra(graphs[ind],0);
    while (!one_step(state)) {
      hydra_to_forest(state,s);
      printf("%s\n",s);
    }
    print_hydra(state);
  }
  return 0;
}
