#include <ucontext.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <time.h>

#define stack_size 1024 * 1024

#define begin ctx->start = clock();\
ctx->total_time = 0

#define yield ctx->end = clock();\
ctx->total_time += ((double)(ctx->end - ctx->start)) / CLOCKS_PER_SEC;\
swapcontext(&ctx->context,&main_ctx);\
ctx->start = clock()

#define finish ctx->finished = true;\
++*num_finished;\
ctx->total_time += ((double)(ctx->end - ctx->start)) / CLOCKS_PER_SEC;\
printf("This coroutine worked for %f ms\n",ctx->total_time*1000);\
swapcontext(&ctx->context,&main_ctx)

ucontext_t main_ctx;

struct coroutine {
    ucontext_t context;
    char *stack;
    bool finished;
    clock_t start;
    clock_t end;
    double total_time;
};