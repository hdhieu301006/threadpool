#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/threadpool.h"

static void *thread_function(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->queued == 0 && !pool->stop) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->stop && pool->queued == 0) {
            pthread_cond_broadcast(&(pool->notify));
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
        pthread_create(&(pool->threads[i]), NULL, thread_function, pool);
    }
}

void threadpool_add_task(threadpool_t *pool, void (*function)(void*), void *arg) {
    if (pool == NULL || function == NULL) {
        return;
    }

    pthread_mutex_lock(&(pool->lock));

    int next_rear = (pool->queue_back + 1) % QUEUE_SIZE;
    if (pool->queued >= QUEUE_SIZE || pool->stop) {
        pthread_mutex_unlock(&(pool->lock));
        return;
    }
        
    pool->task_queue[pool->queue_back].fn = function;
    pool->task_queue[pool->queue_back].arg = arg;
    pool->queue_back = next_rear;
    pool->queued++;
    
    pthread_cond_signal(&(pool->notify));

    pthread_mutex_unlock(&(pool->lock));
}

void threadpool_destroy(threadpool_t *pool) {
    if (pool == NULL) {
        return;
    }
    
    pthread_mutex_lock(&(pool->lock));
    pool->stop = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
}

