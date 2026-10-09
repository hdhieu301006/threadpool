#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/threadpool.h"

void threadpool_init(threadpool_t *pool) {
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

