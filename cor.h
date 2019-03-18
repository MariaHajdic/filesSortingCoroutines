#include <ucontext.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <time.h>

#define stack_size 1024 * 1024

#define begin do { ctx->start = clock();\
ctx->total_time = 0; } while (0)

#define yield do { ctx->end = clock();\
if ((double)(clock() - ctx->start) / CLOCKS_PER_SEC * 1000 < ctx->latency_ms) break;\
ctx->total_time += ((double)(ctx->end - ctx->start)) / CLOCKS_PER_SEC;\
++ctx->swaps_counter;\
swapcontext(&ctx->context,&main_ctx);\
ctx->start = clock(); } while (0)

#define finish do { ctx->finished = true;\
++*num_finished;\
ctx->end = clock();\
ctx->total_time += ((double)(ctx->end - ctx->start)) / CLOCKS_PER_SEC;\
printf("This coroutine worked for %f ms\nNumber of swaps performed: %d\n",ctx->total_time*1000,++ctx->swaps_counter);\
return; } while(0)

ucontext_t main_ctx;

struct coroutine {
    ucontext_t context;
    char *stack;
    bool finished;
    clock_t start;
    clock_t end;
    double total_time;
    int latency_ms;
    int swaps_counter;
};