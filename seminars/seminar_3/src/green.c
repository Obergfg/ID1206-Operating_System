#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;
static green_t *ready_first = NULL;
static green_t *ready_last = NULL;

static void init() __attribute__((constructor));

void init()
{
    getcontext(&main_cntx);
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

    if (!ready_first)
        return NULL;

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

    if (this->join)
        add_ready(this->join);

    this->join = NULL;
    this->retval = result;
    this->zombie = TRUE;
    green_t *next = get_ready();
    running = next;
    setcontext(next->context);
}

int green_create(green_t *new, void *(*fun)(void *), void *arg)
{

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

    return 0;
}

int green_yield()
{
    green_t *susp = running;

    add_ready(susp);

    green_t *next = get_ready();
    running = next;
    swapcontext(susp->context, next->context);

    return 0;
}

int green_join(green_t *thread, void **res)
{

    if (!thread->zombie)
    {
        green_t *susp = running;
        thread->join = susp;
        running = get_ready();
        swapcontext(susp->context, running->context);
    }

    if (res)
        *(res) = thread->retval;

    free(thread->context->uc_stack.ss_sp);
    free(thread->context);

    return 0;
}

void green_cond_init(green_cond_t *cond)
{
    cond->first = cond->last = NULL;
}

void green_cond_wait(green_cond_t *cond)
{
    if(!cond->first)
        cond->first = cond->last = running;
    else
        cond->last = cond->last->next = running;

    green_t *susp = running;
    running = get_ready();
    swapcontext(susp->context, running->context);
}

void green_cond_signal(green_cond_t *cond)
{
    if(!cond->first)
        return;

    add_ready(cond->first);
    cond->first = cond->first->next;
}