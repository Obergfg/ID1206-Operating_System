#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "mag.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

int timers = 0;

static sigset_t block;

#define PERIOD 10
#define FALSE 0
#define TRUE 1
#define UNBLOCK sigprocmask(SIG_UNBLOCK, &block, NULL);
#define BLOCK sigprocmask(SIG_BLOCK, &block, NULL);
#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL,  FALSE};

static green_t *running = &main_green;
green_t *ready_next = NULL;
green_t *ready_end = NULL;
green_t *joining = NULL;

static void init() __attribute__((constructor));
void timer_handler (int);
void green_cond_init(green_cond_t*);
void green_cond_signal(green_cond_t*);
void green_thread();
void addReady(green_t*);
green_t *findNextRunning();

void init() {
  getcontext(&main_cntx);

  sigemptyset(&block);
  sigaddset(&block, SIGVTALRM );
  
  
  struct sigaction act = {0};
  struct timeval interval;
  struct itimerval period;
  act.sa_handler = timer_handler;
  assert(sigaction(SIGVTALRM, &act , NULL) == 0);
  interval.tv_sec = 0;
  interval.tv_usec = PERIOD;
  period.it_interval = interval;
  period.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &period, NULL);
  
  BLOCK
}

void timer_handler(const int sig) {
  timers++;
  green_t* susp = running ;
  addReady(susp);
  running = findNextRunning();
  susp == running || swapcontext(susp->context, running->context);
}


int green_create(green_t *new,void *(* const fun)(void*), void *arg) {  
  BLOCK
  ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
  getcontext(cntx);
  void *stack = malloc(STACK_SIZE);
  cntx->uc_stack.ss_sp = stack;
  cntx->uc_stack.ss_size = STACK_SIZE;
  makecontext(cntx, green_thread, 0);  

  new->context = cntx;
  new->fun = fun;
  new->arg = arg;
  new->next = NULL;
  new->join = NULL;
  new->retval = NULL;
  new->zombie = FALSE;
  addReady(new);
  UNBLOCK
  return 0;
}

void addReady(green_t *rdy) {
    if(!rdy) return; 
    if(!ready_next) ready_end = ready_next = rdy; 
    else ready_end = ready_end->next = rdy;
}

void green_thread() {
  BLOCK
  green_t *this = running;
  UNBLOCK
  this->retval = (*this->fun)(this->arg);
  BLOCK
  this->zombie = TRUE;
  if(this->join) addReady(this->join);
  this->join=NULL;
  green_t* next = findNextRunning();
  running = next;
  setcontext(next->context);
  UNBLOCK
}

green_t* findNextRunning() {
  green_t *current = ready_next;
  if(current == ready_end)
    ready_next = ready_end = NULL;
  else
    ready_next = ready_next->next;
  assert(current && "ERROR, or last thread called wait!!!");
  current->next = NULL;
  return current->zombie ? findNextRunning() : current;
}

int green_yield() {
  BLOCK
  green_t * susp = running;
  addReady(susp);
  running = findNextRunning();
  assert(running != NULL);
  swapcontext(susp->context, running->context);
  UNBLOCK
  return 0;
}

int green_join(green_t *thread, void **res) {
  BLOCK
  if(!thread->zombie) {
    green_t *susp = running;
    thread->join = susp;
    running = findNextRunning();
    swapcontext(susp->context, running->context);
  }
  
  if(res) *(res) = (thread->retval);
  free((void*)thread->context->uc_stack.ss_sp);
  free(thread->context);
  UNBLOCK
  return 0;  
}

int green_cond_wait(green_cond_t *cond, green_mutex_t *mutex) {
  BLOCK
  if(mutex) {
    if(mutex->start) {
      green_t* awoken = mutex->start;
      mutex->start = mutex->start->next;
      awoken->next = NULL;
      addReady(awoken);
    } else
      mutex->taken=FALSE;
  }

  if(!cond->start)
    cond->start = cond->end = running;
  else 
    cond->end = cond->end->next = running;
  
  green_t* susp = running;
  running = findNextRunning();
  swapcontext(susp->context, running->context);

  if(mutex) {
    if(mutex->taken) {
      green_t *susp = running;
      if(!mutex->start)
        mutex->start = mutex->end = running; 
      else 
        mutex->end = mutex->end->next = running;
      running = findNextRunning();
      swapcontext(susp->context, running->context);
    } else {
      mutex->taken = TRUE;
    }
  }
  UNBLOCK
  return 0;
}

int green_mutex_init(green_mutex_t *mutex) {
  mutex->taken = FALSE;
  mutex->start = mutex->end = NULL;
}

int green_mutex_lock(green_mutex_t *mutex) {
  BLOCK 
  if(mutex->taken) {
    green_t *susp = running;
    if(!mutex->start)
      mutex->start = mutex->end = running; 
    else 
      mutex->end = mutex->end->next = running;
    running = findNextRunning();
    swapcontext(susp->context, running->context);
  } else 
    mutex->taken = TRUE;
  UNBLOCK
  return 0;
}

int green_mutex_unlock(green_mutex_t *mutex) {
  BLOCK 
  if(mutex->start) {
    green_t* awoken = mutex->start;
    mutex->start = mutex->start->next;
    awoken->next = NULL;
    addReady(awoken);
  } else
    mutex->taken=FALSE;
  UNBLOCK
  return 0;
}

void green_cond_init(green_cond_t* cond) {
  //cond = (green_cond_t*) malloc(sizeof(green_cond_t));
  cond->end = cond->start = NULL;
}

void green_cond_wait_DEPR(green_cond_t* cond) {
  BLOCK
  if(!cond->start)
    cond->start = cond->end = running; 
  else 
    cond->end = cond->end->next = running;
  green_t* susp = running;
  running = findNextRunning();
  swapcontext(susp->context, running->context);
  UNBLOCK
}

void green_cond_signal(green_cond_t* cond) {
  BLOCK
  if(!cond->start) return;
  addReady(cond->start);
  cond->start = cond->start->next;
  ready_end->next = NULL; //saves 2 lines...
  UNBLOCK
}