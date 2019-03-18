#include "merge_sort.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "aio.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "unistd.h"

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

int async_fread(struct coroutine *ctx, char* fname, array *input) {
    int f = open(fname,S_IREAD); yield;
    if (f == -1) return -1;

    char buff[256]; 
    struct aiocb params; 
    params.aio_fildes = f; yield;
    params.aio_offset = 0; yield;
    params.aio_lio_opcode = LIO_READ; yield;
    params.aio_nbytes = 256; yield;
    params.aio_buf = buff; yield;

    int number = 0, data_size = 4;
    input->data = malloc(data_size*sizeof(int)); yield;
    input->len = 0; yield;
    while (true) {
        if (aio_read(&params) != 0) return -1;
        while (aio_error(&params) == EINPROGRESS) {
            yield;
        }
        int bytes_read = aio_return(&params); yield;
        if (bytes_read == 0) break;

        params.aio_offset += bytes_read; yield;
        for (int i = 0; i < bytes_read; ++i) {
            if (buff[i] == ' ' || buff[i] == EOF) {
                input->data[input->len++] = number; yield;
                number = 0; yield;
                if (input->len >= data_size) {
                    int *temp = input->data; yield;
                    input->data = malloc(data_size*2*sizeof(int)); yield;
                    memcpy(input->data,temp,data_size*sizeof(int)); yield;
                    free(temp); yield;
                    data_size *= 2; yield;
                }
                continue;
            }
            number = number * 10 + (buff[i] - '0'); yield;
        }
    }
    close(f); yield;
    return 0;
}

void sort_init(struct coroutine *ctx, char* fname, int *num_finished, array *result) {
    begin;
    array buff; 
    if (async_fread(ctx,fname,&buff) == -1) {
        printf("Unable to read file\n"); finish;
    }
    result->data = malloc(buff.len*sizeof(int)); yield;
    result->len = buff.len; yield;
    merge_sort(buff,*result,ctx); yield;
    free(buff.data); yield;
    finish;
}