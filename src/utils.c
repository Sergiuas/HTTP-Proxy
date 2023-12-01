#include "utils.h"

ThreadPool *thread_pool_init(int thread_count) {
  ThreadPool *pool = malloc(sizeof(ThreadPool));
  if (pool == NULL) {
    return NULL;
  }

  pool->threads = malloc(thread_count * sizeof(pthread_t));
  if (pool->threads == NULL) {
    free(pool);
    return NULL;
  }

  pool->thread_count = thread_count;
  pool->tasks = NULL;
  pool->task_count = 0;
  pool->task_queue_size = 0;
  pool->task_queue_front = 0;
  pool->task_queue_rear = 0;
  pool->shutdown = false;

  if (pthread_mutex_init(&(pool->mutex), NULL) != 0) {
    free(pool->threads);
    free(pool);
    return NULL;
  }

  if (pthread_cond_init(&(pool->cond), NULL) != 0) {
    pthread_mutex_destroy(&(pool->mutex));
    free(pool->threads);
    free(pool);
    return NULL;
  }

  //   for (int i = 0; i < thread_count; i++) {
  //     if (pthread_create(&(pool->threads[i]), NULL, thread_function, (void
  //     *)pool) != 0) {
  //       thread_pool_destroy(pool);
  //       return NULL;
  //     }
  //   }

  return pool;
}
