#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096
#define PERIOD 100

static void init() __attribute__((constructor));
void timer_handler(int);

static sigset_t block;
static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;
static green_t *ready_first = NULL;
static green_t *ready_last = NULL;

void init()
{
    getcontext(&main_cntx);
    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;

    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM, &act, NULL) == 0);

    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
}

void add_ready(green_t *new)
{
    if (!new)
        return;

    if (!ready_first)
        ready_first = ready_last = new;
    else
        ready_last = ready_last->next = new;
}

green_t *get_ready()
{
    green_t *thread = ready_first;

    if (thread == ready_last)
        ready_first = ready_last = NULL;
    else
        ready_first = ready_first->next;

    thread->next = NULL;
    return thread;
}

void green_thread()
{  
    green_t *this = running;

    void *result = (*this->fun)(this->arg);

    sigprocmask(SIG_BLOCK, &block, NULL);
    if (this->join)
        add_ready(this->join);

    this->join = NULL;
    this->retval = result;
    this->zombie = TRUE;
    assert(ready_first != NULL);
    green_t *next = get_ready();
    running = next;
    assert(running != NULL);
    setcontext(next->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_create(green_t *new, void *(*fun)(void *), void *arg)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
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

    add_ready(new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_yield()
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    green_t *susp = running;
    add_ready(susp);
    assert(ready_first != NULL);
    running = get_ready();

    /*
        Forcing timer interrupt in function manipulating state of threads.
        Need to uncomment this if-block as well as comment out the sigprocmask 
        calls in this function for it to work.
    */
    //  if(interrupts == 20)
    //      timer_handler(12);

    swapcontext(susp->context, running->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

int green_join(green_t *thread, void **res)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (!thread->zombie)
    {
        green_t *susp = running;
        thread->join = susp;
        assert(ready_first != NULL);
        running = get_ready();
        assert(running != NULL);
        swapcontext(susp->context, running->context);
    }

    if (res)
        *(res) = thread->retval;

    free(thread->context->uc_stack.ss_sp);
    free(thread->context);

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

/* CONDITIONAl */

void green_cond_init(green_cond_t *cond)
{
    cond->first = cond->last = NULL;
}

void green_cond_wait_old(green_cond_t *cond)
{
    if (!cond->first)
        cond->first = cond->last = running;
    else
        cond->last = cond->last->next = running;

    green_t *susp = running;
    assert(ready_first != NULL);
    running = get_ready();
    swapcontext(susp->context, running->context);
}

void green_cond_signal(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (!cond->first)
        return;

    add_ready(cond->first);
    cond->first = cond->first->next;
    ready_last->next = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_broadcast(green_cond_t *cond){
    
    sigprocmask(SIG_BLOCK, &block, NULL);
    
    if (!cond->first)
        return;
    else{
        green_t *ready = cond->first;
       
        do{
            add_ready(ready);
            ready = ready->next;
        }while(ready);
    }

    cond->first = cond->last = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);    
}

/* TIMER */

void timer_handler(int sig)
{
    interrupts++;
    green_t *susp = running;
    add_ready(susp);
    assert(ready_first != NULL);
    running = get_ready();
    swapcontext(susp->context, running->context);
}

/* MUTEX */

int green_mutex_init(green_mutex_t *mutex)
{
    mutex->taken = FALSE;
    mutex->first = mutex->last = NULL;
}

int green_mutex_lock(green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (mutex->taken)
    {
        green_t *susp = running;

        if (!mutex->first)
            mutex->first = mutex->last = susp;
        else
            mutex->last = mutex->last->next = susp;

        assert(ready_first != NULL);
        running = get_ready();
        swapcontext(susp->context, running->context);
    }
    else
        mutex->taken = TRUE;

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (mutex->first)
    {
        green_t *ready = mutex->first;
        mutex->first = mutex->first->next;
        ready->next = NULL;
        add_ready(ready);
    }
    else
        mutex->taken = FALSE;

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_cond_wait(green_cond_t *cond, green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (mutex)
    {
        if (mutex->first)
        {
            green_t *ready = mutex->first;
            mutex->first = mutex->first->next;
            ready->next = NULL;
            add_ready(ready);
        }
        else
            mutex->taken = FALSE;
    }

    if (!cond->first)
        cond->first = cond->last = running;
    else
        cond->last = cond->last->next = running;

    green_t *susp = running;
    assert(ready_first != NULL);
    running = get_ready();
  
    swapcontext(susp->context, running->context);

    if (mutex)
    {
        if (mutex->taken)
        {
            green_t *susp = running;

            if (!mutex->first)
                mutex->first = mutex->last = running;
            else
                mutex->last = mutex->last->next = running;

            assert(ready_first != NULL);
            running = get_ready();
            swapcontext(susp->context, running->context);
        }
        else
            mutex->taken = TRUE;
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}
