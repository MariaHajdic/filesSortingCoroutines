#include "cor.h"

typedef struct array {
    int* data;
    int len;
} array;

void merge_results(array *results, int n);
void merge(array a1, array a2, array res, struct coroutine *ctx);
void merge_sort(array arr, array res, struct coroutine *ctx);
void sort_init(struct coroutine *ctx, char* fname, int *num_finished, array *result);