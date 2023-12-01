
#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

// Function to initialize a thread pool
ThreadPool *thread_pool_init(int thread_count);

// Thread function to execute the tasks in the queue:
void *thread_function(void *arg);

// Function to add a task to the thread pool
void thread_pool_add_task(ThreadPool *pool, void (*task)(void *), void *arg);

// Function to shutdown the thread pool
void thread_pool_shutdown(ThreadPool *pool);

// Function to create a server socket
int create_server_socket(int port);

// Function to accept client connections
int accept_client_connection(int server_socket);

// Function to receive data from a socket
int receive_data(int socket, void *buffer, int buffer_size);

// Function to send data to a socket
int send_data(int socket, const void *buffer, int buffer_size);

//

#endif // UTILS_H
