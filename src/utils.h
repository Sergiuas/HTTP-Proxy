
#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

void setnonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

#define TASK_QUEUE_SIZE 100
#define THREAD_MAX_COUNT 100

// Task structure for a thread:
typedef struct Task {
  void (*function)(void *); // Pointer to the function to be executed
  void *argument;           // Argument to be passed to the function
} Task;

// Thread pool structure
typedef struct ThreadPool {
  pthread_t *threads;    // Array of thread IDs
  int thread_count;      // Number of threads in the pool
  Task *tasks;           // Array of tasks
  int task_count;        // Number of tasks in the pool
  int task_queue_size;   // Size of the task queue
  int task_queue_front;  // Index of the front of the task queue
  int task_queue_rear;   // Index of the rear of the task queue
  bool shutdown;         // Flag to indicate if the pool should be shutdown
  pthread_mutex_t mutex; // Mutex for thread synchronization
  pthread_cond_t done;   // Condition variable for thread synchronization
  pthread_cond_t cond;   // Condition variable for thread synchronization
} ThreadPool;


typedef struct ClientRequest{
    int socketfd;
    char* request;
} ClientRequest;

 
size_t b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}


char *b64_encode(const unsigned char *in, size_t len)
{
  const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0)
		return NULL;

	elen = b64_encoded_size(len);
	out  = malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}

	return out;
}

// Function to initialize a thread pool
ThreadPool *thread_pool_init(int thread_count);

// Thread function to execute the tasks in the queue:
void *thread_function(void *arg);

// Function to add a task to the thread pool
void thread_pool_add_task(ThreadPool *pool, void (*task)(void *), void *arg);

// Function to shutdown the thread pool
void thread_pool_shutdown(ThreadPool *pool);

// Function to accept client connections
int accept_client_connection(int server_socket);

// Function to receive data from a socket
int receive_data(int socket, void *buffer, int buffer_size);

// Function to send data to a socket
int send_data(int socket, const void *buffer, int buffer_size);

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

    printf("%d numar\n", pool->task_queue_front);

    // Update the queue information
    pool->task_count--;
    pool->task_queue_front = pool->task_queue_front + 1;

    // Signal waiting threads that a task has been completed
    pthread_cond_signal(&(pool->done));

    pthread_mutex_unlock(&(pool->mutex));

    // Execute the task from the thread pool
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
  int next_index = pool->task_queue_rear;
  pool->tasks[next_index].function = task;
  pool->tasks[next_index].argument = arg;
  pool->task_count++;
  pool->task_queue_rear = next_index + 1;

  printf("%d numar index\n", next_index);
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

#endif // UTILS_H
