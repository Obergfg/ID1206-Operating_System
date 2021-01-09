#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "green.h"

#define MAX 500

typedef struct cell {
  int val;
  struct cell *next;
} cell;

cell sentinel = {MAX, NULL};
cell dummy = {-1, &sentinel};
cell *global = &dummy;

green_mutex_t mutex;

void toggle(cell *lst, int r) {

  cell *prev = NULL;
  cell *this = lst;
  cell *removed = NULL;

  green_mutex_lock(&mutex);
  
  while(this->val < r) {
      prev = this;
      this = this->next;
  }

  if(this->val == r) {
    prev->next = this->next;
    removed = this;
  } else {
    cell *new = malloc(sizeof(cell));
    new->val = r;
    new->next = this;
    prev->next = new;
  }

  green_mutex_unlock(&mutex);   

  if(removed != NULL)  
    free(removed);

  return;
}

typedef struct args {
    int inc; 
    int id; 
    cell *list;
} args;

void *bench(void *arg) {

  int inc = ((args*)arg)->inc;
  int id = ((args*)arg)->id;
  cell *lstp = ((args*)arg)->list;  

  for(int i = 0; i < inc; i++) {
    int r = rand() % MAX;
    toggle(lstp, r);
  }
}

void printlist(void* arg){

    cell *list = (cell*) arg;

    while (list->val < MAX){
      printf("%d ", list->val);
      list = list->next;
    }
}

int main(int argc, char *argv[]) {

  if(argc != 3) {
    printf("usage: list <total> <threads>\n");
    exit(0);
  }

  int n = atoi(argv[2]);  
  int inc = (atoi(argv[1]) / n);
  
  green_mutex_init(&mutex);

  args *thra = malloc(n * sizeof(args));

  for(int i =0; i < n; i++) {
    thra[i].inc = inc;
    thra[i].id = i;
    thra[i].list = global;
  }

  green_t *thrt = malloc(n * sizeof(green_t));  

  struct timespec t_start, t_stop;

  clock_gettime(CLOCK_MONOTONIC_COARSE, &t_start);

  for(int i =0; i < n; i++) {
    green_create(&thrt[i], bench, &thra[i]);
  }

  for(int i =0; i < n; i++) {
    green_join(&thrt[i], NULL);
  }  

  clock_gettime(CLOCK_MONOTONIC_COARSE, &t_stop);

  long wall_sec = t_stop.tv_sec - t_start.tv_sec;
  long wall_nsec = t_stop.tv_nsec - t_start.tv_nsec;
  long wall_msec = (wall_sec *1000) + (wall_nsec / 1000000);
  
  printf("%ld\n", wall_msec);

 
  
  return 0;
}