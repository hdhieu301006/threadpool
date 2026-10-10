#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/threadpool.h"

void example_task(void *arg) {
    int *num = (int *)arg;
    printf("Processing task %d\n", *num);
    sleep(1);
    free(arg);
}

int main(void) {
    threadpool_t pool;
    threadpool_init(&pool);

    for (int i = 0; i < 100; i++) {
        int *task_num = malloc(sizeof(int));
        if (task_num == NULL) {
            continue;
        }
        *task_num = i;
        threadpool_add_task(&pool, example_task, task_num);
    }

    threadpool_destroy(&pool);

    return 0;
}