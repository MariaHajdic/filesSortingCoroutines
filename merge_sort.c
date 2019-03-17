#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "merge_sort.h"

void merge_results(array *results, int n) {
    FILE *fres = fopen("res.txt","w");
    if (!fres) {
        printf("Unable to create result file\n");
        return;
    }

    int *idxs = (int*)malloc(n*sizeof(int)); 
    memset(idxs,0,n*sizeof(int));

    int total_len = 0;
    for (int i = 0; i < n; ++i) {
        total_len += results[i].len;
    }

    int j = 0;
    while (j++ < total_len) {
        int min_idx = 0;
        for (int i = 0; i < n; ++i) {
            if (idxs[i] < results[i].len) {
                min_idx = i;
                break;
            }
        }
        for (int i = 0; i < n; ++i) {
            if (idxs[i] >= results[i].len) continue;
            if (results[i].data[idxs[i]] < results[min_idx].data[idxs[min_idx]]) 
                min_idx = i;
        }
        int current_min = results[min_idx].data[idxs[min_idx]++];
        fprintf(fres,"%d ",current_min);
    }

    free(idxs);
    fclose(fres);    
}

void merge(array a1, array a2, array res, struct coroutine *ctx) {
    int idx_1 = 0, idx_2 = 0, i = 0; yield;
    while (idx_1 < a1.len && idx_2 < a2.len) {
        res.data[i++] = a1.data[idx_1] < a2.data[idx_2] ? a1.data[idx_1++] : a2.data[idx_2++]; yield;
    }
    if (idx_1 < a1.len) memcpy(&res.data[i],&a1.data[idx_1],(a1.len - idx_1)*sizeof(int));
    if (idx_2 < a2.len) memcpy(&res.data[i],&a2.data[idx_2],(a2.len - idx_2)*sizeof(int));
    yield;
}

void merge_sort(array arr, array res, struct coroutine *ctx) {
    if (arr.len == 1) {
        res.data[0] = arr.data[0]; 
        return;
    }

    array left, right;
    left.len = arr.len / 2; yield;
    left.data = arr.data; yield;
    right.len = arr.len - left.len; yield;
    right.data = arr.data + left.len; yield;

    array left_res, right_res;
    left_res.len = left.len; yield;
    right_res.len = right.len; yield;
    left_res.data = res.data; yield;
    right_res.data = res.data + left_res.len; yield;

    merge_sort(left,left_res,ctx); yield;
    merge_sort(right,right_res,ctx); yield;

    array tmp;
    tmp.data = malloc(res.len*sizeof(int)); yield;
    tmp.len = res.len; yield;
   
    merge(left_res,right_res,tmp,ctx); yield;

    memcpy(res.data, tmp.data, res.len*sizeof(int)); yield;
    free(tmp.data); yield;
}

void sort_init(struct coroutine *ctx, char* fname, int *num_finished, array *result) {
    begin;
    FILE* fptr = fopen(fname,"r"); yield;
    if (fptr == NULL) {
        printf("File not opened/n"); finish;
    } 

    array buff;
    int tmp[1024*50];
    buff.data = tmp; yield;
    buff.len = 0; yield;

    while (!feof(fptr)) {
        fscanf(fptr,"%d",&buff.data[buff.len++]); yield;
    }
    fclose(fptr); yield;

    result->data = malloc(buff.len*sizeof(int)); yield;
    result->len = buff.len; yield;
    merge_sort(buff,*result,ctx); yield;
    finish;
}