#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/threadpool.h"

void *thread_function(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->queued == 0 && !pool->stop) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->stop && pool->queued == 0) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % QUEUE_SIZE;
        pool->queued--;

        pthread_mutex_unlock(&(pool->lock));

        task.fn(task.arg);
    }

    return NULL;
}

void threadpool_init(threadpool_t *pool) {
    if (pool == NULL) {
        return;
    }
    
    pool->queued = 0;
    pool->queue_front = 0;
    pool->queue_back = 0;
    pool->stop = 0;

    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);
    
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, thread_function, pool);
    }
}

