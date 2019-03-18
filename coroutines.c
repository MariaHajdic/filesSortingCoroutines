#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void *allocate_stack() {
    return mmap(NULL, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

int main(int argc, char *argv[]) {
    time_t begin_main = clock();
    const int n = argc - 2;
    int num_finished = 0;
    int latency = atoi(argv[1]);

    struct coroutine *coroutines = (struct coroutine*)malloc(n*sizeof(struct coroutine));
    array *sorted_files = (array*)malloc(n*sizeof(array));

    for (int i = 0; i < n; ++i) {
        coroutines[i].stack = allocate_stack();
        if (getcontext(&coroutines[i].context) == -1) return 1;
        coroutines[i].context.uc_stack.ss_sp = coroutines[i].stack;
        coroutines[i].context.uc_stack.ss_size = stack_size;
        coroutines[i].context.uc_link = &main_ctx;
        coroutines[i].finished = false;
        coroutines[i].swaps_counter = 0;
        coroutines[i].latency_ms = latency / n;
        makecontext(&coroutines[i].context,sort_init,4,&coroutines[i],argv[i+2],&num_finished,&sorted_files[i]);
    }
    
    int i = -1;
    while (num_finished < n) {
        i = (i + 1) % n;
        if (coroutines[i].finished) continue;
        if (swapcontext(&main_ctx,&coroutines[i].context) == -1)
            handle_error("swapcontext");
    }
    
    merge_results(sorted_files,n);
    for (int i = 0; i < n; ++i) {
        free(sorted_files[i].data);
    }
    free(sorted_files);
    for (int i = 0; i < n; ++i) {
        munmap(coroutines[i].stack,stack_size);
    }
    free(coroutines);

    time_t end = clock();
    printf("Total program working time: %f ms\n",((double)(end - begin_main)) / CLOCKS_PER_SEC * 1000);

    return 0;
}