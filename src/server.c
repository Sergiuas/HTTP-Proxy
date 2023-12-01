#include "utils.h"

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

  message = "Now type something, and I shall repeat what you type \n";
  write(client_sock, message, strlen(message));

  // Receive message from client socket:
  while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0) {
    // Send the message back to the client:
    write(client_sock, client_message, strlen(client_message));

    memset(client_message, '\0', sizeof(client_message));
  }

  if (read_size == 0) {
    printf("Client disconnected\n");
    fflush(stdout);
  } else if (read_size == -1) {
    printf("Error while receiving client message\n");
  }

  // Close the socket:
  close(client_sock);

  return NULL;
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
  ThreadPool *pool;
  pool = thread_pool_init(5);

  // Verify if the thread pool was created successfully:
  if (pool == NULL) {
    printf("Error while initializing thread pool\n");
    return -1;
  }

  // While the server is running, accept connections from clients and assign to
  // threads to handle:
  int thread_index = 0;
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
    pthread_create(pool->threads[thread_index], NULL, handle_client,
                   (void *)&client_sock);
    pthread_detach(pool->threads[thread_index]);

    // Increment the thread index for the next client:
    thread_index = (thread_index + 1) % pool->thread_count;

    // Closing the socket:
    // close(client_sock);
  }

  // Closing the socket:
  close(socket_desc);

  return 0;
}