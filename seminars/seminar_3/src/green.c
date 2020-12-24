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
static green_t *first_ready = NULL;
static green_t *last_ready = NULL;
static queue ready = {NULL, NULL};

static void init() __attribute__((constructor));

void init()
{
    getcontext(&main_cntx);
}

void add(green_t *new)
{

    if (!new)
        return;

    if (!first_ready)
         first_ready = last_ready = new;
    else
    {
        last_ready->next = new;
        last_ready = new;
    }
}

green_t *get_first()
{

    if (!first_ready)
        return NULL;

    green_t *ready = first_ready;

    if(!first_ready->next)
        first_ready = last_ready;
    else
        first_ready = first_ready->next;

    return ready;
}

void green_thread()
{
    green_t *this = running;

    void *result = (*this->fun)(this->arg);

    if (this->join)
        add(this->join);

    this->join = NULL;
    this->retval = result;
    this->zombie = TRUE;
    green_t *next = get_first();
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

    add(new);

    return 0;
}

int green_yield()
{
    green_t *susp = running;

    add(susp);

    green_t *next = get_first();
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
        green_t *next = get_first();
        running = next;
        swapcontext(susp->context, next->context);
    }
    // collect result
    if (res)
        *(res) = thread->retval;

    // free context
    free(thread->context->uc_stack.ss_sp);
    free(thread->context);

    return 0;
}