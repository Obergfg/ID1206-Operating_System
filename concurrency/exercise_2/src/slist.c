#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define MAX 10000

typedef struct cell {
  int val;
  struct cell *next;
  int mutex;
} cell;

cell sentinel = {MAX, NULL, 0};
cell dummy = {-1, &sentinel, 0};
cell *global = &dummy;

typedef struct args {
    int inc; 
    int id; 
    cell *list;
} args;

int try(int *mutex) {
  return __sync_val_compare_and_swap(mutex, 0, 1);
}

void lock(int *mutex) {
  while(try(mutex) != 0) {};
}

void unlock(int *mutex) {
  *mutex = 0;
}

void toggle(cell *lst, int r) {

  cell *prev = lst;
  lock(&prev->mutex); 

  cell *this = prev->next;
  lock(&this->mutex);

  cell *removed = NULL;

  while(this->val < r) {
    unlock(&prev->mutex);      
    prev = this;
    this = this->next;
    lock(&this->mutex);      
  }

  if(this->val == r) {
    prev->next = this->next;
    removed = this;
  } else {
    cell *new = malloc(sizeof(cell));
    new->val = r;
    new->next = this;
    new->mutex = 0;
    prev->next = new;
  }

   unlock(&prev->mutex);      
   unlock(&this->mutex);   

   if(removed != NULL) 
     free(removed);

   return;
}

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

  printf("%d threads doing %d operations each\n", n, inc);
  

  args *thra = malloc(n * sizeof(args));

  for(int i =0; i < n; i++) {
    thra[i].inc = inc;
    thra[i].id = i;
    thra[i].list = global;
  }

  pthread_t *thrt = malloc(n * sizeof(pthread_t));  

  struct timespec t_start, t_stop;

  clock_gettime(CLOCK_MONOTONIC_COARSE, &t_start);

  for(int i =0; i < n; i++) {
    pthread_create(&thrt[i], NULL, bench, &thra[i]);
  }

  for(int i =0; i < n; i++) {
    pthread_join(thrt[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC_COARSE, &t_stop);

  long wall_sec = t_stop.tv_sec - t_start.tv_sec;
  long wall_nsec = t_stop.tv_nsec - t_start.tv_nsec;
  long wall_msec = (wall_sec *1000) + (wall_nsec / 1000000);
  
  printf("done in %ld ms\n", wall_msec);

//   printf("The created list\n");
//   printlist(global);
  
  return 0;
}