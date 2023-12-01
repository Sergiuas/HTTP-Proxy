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
  pool->tasks = malloc(TASK_QUEUE_SIZE * sizeof(Task));
  pool->task_count = 0;
  pool->task_queue_size = TASK_QUEUE_SIZE;
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

  if (pthread_cond_init(&(pool->done), NULL) != 0) {
    pthread_mutex_destroy(&(pool->mutex));
    free(pool->threads);
    free(pool);
    return NULL;
  }

  for (int i = 0; i < thread_count; i++) {
    if (pthread_create(&(pool->threads[i]), NULL, thread_function,
                       (void *)pool) != 0) {
      thread_pool_shutdown(pool);
      return NULL;
    }
  }

  return pool;
}

// Create a thread function to execute the tasks in the queue:
void *thread_function(void *arg) {
  ThreadPool *pool = (ThreadPool *)arg;
  Task task;

  while (true) {
    pthread_mutex_lock(&(pool->mutex));

    // Wait on condition variable if the queue is empty
    while (pool->task_count == 0 && !(pool->shutdown)) {
      pthread_cond_wait(&(pool->cond), &(pool->mutex));
    }

    // Shutdown the thread if the shutdown flag is set
    if (pool->shutdown) {
      pthread_mutex_unlock(&(pool->mutex));
      pthread_exit(NULL);
    }

    // Copy the task from the queue
    task.function = pool->tasks[pool->task_queue_front].function;
    task.argument = pool->tasks[pool->task_queue_front].argument;

    // Update the queue information
    pool->task_count--;
    pool->task_queue_front =
        (pool->task_queue_front + 1) % pool->task_queue_size;

    // Signal waiting threads that a task has been completed
    pthread_cond_signal(&(pool->done));

    pthread_mutex_unlock(&(pool->mutex));

    // Execute the task
    (*(task.function))(task.argument);
  }

  pthread_exit(NULL);
}

void thread_pool_add_task(ThreadPool *pool, void (*task)(void *), void *arg) {

  pthread_mutex_lock(&(pool->mutex));

  // Check if the pool is already shutdown
  if (pool->shutdown) {
    pthread_mutex_unlock(&(pool->mutex));
    return;
  }

  // Check if the task queue is full
  if (pool->task_count == pool->task_queue_size) {
    pthread_mutex_unlock(&(pool->mutex));
    return;
  }

  // Add the task to the task queue
  int next_index = (pool->task_queue_rear + 1) % pool->task_queue_size;
  pool->tasks[next_index].function = task;
  pool->tasks[next_index].argument = arg;
  pool->task_count++;
  pool->task_queue_rear = next_index;

  // Signal a waiting thread that a new task is available
  pthread_cond_signal(&(pool->cond));

  pthread_mutex_unlock(&(pool->mutex));
}

// Function to shutdown the thread pool
void thread_pool_shutdown(ThreadPool *pool) {
  pthread_mutex_lock(&(pool->mutex));

  // Set the shutdown flag to true
  pool->shutdown = true;

// Check if there are still tasks in the queue:
#if 0
    if (pool->task_count > 0) {
        // Signal all the waiting threads
        pthread_cond_broadcast(&(pool->cond));
    }
#endif

  // Signal all the waiting threads
  pthread_cond_broadcast(&(pool->done));

  pthread_mutex_unlock(&(pool->mutex));

  // Wait for all the threads to finish
  for (int i = 0; i < pool->thread_count; i++) {
    pthread_detach(pool->threads[i]);
  }

  // Free the memory allocated for the thread pool
  free(pool->threads);
  free(pool->tasks);
  free(pool);
}

// Function to create a server socket
int create_server_socket(int port) {
  int socket_desc;
  struct sockaddr_in server_addr;

  // Create socket, we use SOCK_STREAM for TCP
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0) {
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  // Set port and IP the same as client-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

// Function to receive data from a socket
int receive_data(int socket, void *buffer, int buffer_size) {
  int data_size;

  // Receive data from client:
  data_size = recv(socket, buffer, buffer_size, 0);

  if (data_size < 0) {
    printf("Error while receiving client message\n");
    return -1;
  }

  return data_size;
}

// Function to send data to a socket
int send_data(int socket, const void *buffer, int buffer_size) {
  int data_size;

  // Send the message to client:
  data_size = send(socket, buffer, buffer_size, 0);

  if (data_size < 0) {
    printf("Unable to send message\n");
    return -1;
  }

  return data_size;
}