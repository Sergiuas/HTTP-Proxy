#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Create a thread pool structure:
typedef struct thread_pool {
  int num_threads;
  pthread_t *threads;
} thread_pool_t;

// Create a function to initialize the thread pool:
void init_thread_pool(thread_pool_t *pool, int num_threads) {
  pool->num_threads = num_threads;
  pool->threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
}

// Create listening socket:
int create_listening_socket(void) {
  int socket_desc;
  struct sockaddr_in server_addr;
  uint16_t port = 8080;

  // Create socket, we use SOCK_STREAM for TCP:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0) {
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  int setopt = 1;
  // Reuse the port. Otherwise, on restart, port 8000 is usually still occupied
  // for a bit and we need to start at another port.
  if (-1 == setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&setopt,
                       sizeof(setopt))) {
    error("ERROR setting socket options");
  }

  // Set port and IP that we'll be listening for, any other IP_SRC or port will
  // be dropped:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Bind to the set port and IP:
  if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Listen for clients. We allow at most 5 clients in the queue:
  if (listen(socket_desc, 5) < 0) {
    printf("Error while listening\n");
    return -1;
  }
  printf("Listening for incoming connections.....\n");

  return socket_desc;
}

// Create a function to handle the client requests:
void *handle_client(void *socket_desc) {
  // Get the socket descriptor:
  int client_sock = *(int *)socket_desc;
  int read_size;
  char *message, client_message[2000];

  // Send message to the client socket:
  message = "Greetings! I am your connection handler\n";
  write(client_sock, message, strlen(message));

  message = "Now type something and i shall repeat what you type \n";
  write(client_sock, message, strlen(message));

  // Receive message from client socket:
  while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0) {
    // Send the message back to client:
    write(client_sock, client_message, strlen(client_message));

    memset(client_message, '\0', sizeof(client_message));
  }

  if (read_size == 0) {
    printf("Client disconnected\n");
    fflush(stdout);
  } else if (read_size == -1) {
    printf("Error while receiving client message\n");
  }

  // Free the socket pointer:
  free(socket_desc);

  return 0;
}

int main(void) {
  int socket_desc, client_sock, client_size;
  struct sockaddr_in server_addr, client_addr;
  char server_message[2000], client_message[2000];

  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  socket_desc = create_listening_socket();
  if (socket_desc < 0) {
    return -1;
  }

  client_size = sizeof(client_addr);

  // Initialize thread pool:
  thread_pool_t pool;
  init_thread_pool(&pool, 5);

  // While the server is running, accept connections from clients and assign to
  // threads to handle:
  while (1) {
    // Accept connection from client:
    client_sock =
        accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

    if (client_sock < 0) {
      printf("Can't accept\n");
      return -1;
    }
    printf("Client connected at IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Assign the client_sock to a thread to handle:
    pthread_create(&pool.threads[0], NULL, handle_client, (void *)client_sock);
    pthread_join(pool.threads[0], NULL);

    // Closing the socket:
    close(client_sock);
  }

  // Closing the socket:
  close(socket_desc);

  return 0;
}