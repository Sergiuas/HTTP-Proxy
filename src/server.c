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

// Create a function to parse a http request:
void parse_request(char *buffer, char *hostname, char *path) {
  char *token = strtok(buffer, " \r\n");
  while (token != NULL) {
    if (strcmp(token, "GET") == 0) { // TODO: Add support for other methods
      token = strtok(NULL, " \r\n");
      if (token != NULL) {
        strcpy(path, token);
      }
    } else if (strcmp(token, "Host:") == 0) {
      token = strtok(NULL, " \r\n");
      if (token != NULL) {
        strcpy(hostname, token);
      }
    }
    token = strtok(NULL, " \r\n");
  }
}

// Create a function to handle the client requests:
void *handle_client(void *socket_desc) {
  // Get the socket descriptor:
  int client_sock = *(int *)socket_desc;
  int read_size;
  char client_message[2000];

  printf("Thread #%ld\n", pthread_self()); // Print the thread id;

  read_size = recv(client_sock, client_message, 2000, 0);

  if (read_size == 0) {
    printf("Client disconnected\n");
    fflush(stdout);
  } else if (read_size == -1) {
    printf("Error while receiving client message\n");
  }

  // Print the received message with write to stdout:
  write(STDOUT_FILENO, client_message, strlen(client_message));

  // Process the request
  char hostname[100];
  char path[200];
  char to_find[2000];

  strcpy(to_find, client_message);
  parse_request(client_message, hostname, path);

  printf("\nHostname: %s\n", hostname);
  printf("Path: %s\n", path);

  // Take the first line from the request:
  char *first_line = strtok(to_find, "\r\n");
  if (first_line == NULL) {
    printf("Error while parsing request\n");
    return NULL;
  }

  printf("First line: %s\n", first_line);
  // Add two /r/n to the end of the first line:
  strcat(first_line, "\r\n\r\n");

  // Take the port number from the hostname, if any, else use port 80:
  strcpy(to_find, hostname);
  char *port = strstr(to_find, ":");
  if (port != NULL) {
    *port = '\0';
    port++;
  } else {
    port = "80";
  }

  printf("Port: %s\n", port);

  // Get the IP address of the hostname:
  // struct hostent *host = gethostbyname(hostname);
  // if (host == NULL) {
  //   printf("Error while getting IP address\n");
  //   return NULL;
  // }

  // printf("IP address: %s\n", inet_ntoa(*((struct in_addr *)host->h_addr)));

  // // Create a socket to connect to the server:
  // int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  // if (server_sock < 0) {
  //   printf("Error while creating socket\n");
  //   return NULL;
  // }

  // // Connect to the server:
  // struct sockaddr_in server_addr;
  // server_addr.sin_family = AF_INET;
  // server_addr.sin_port = htons(atoi(port));
  // server_addr.sin_addr = *((struct in_addr *)host->h_addr);
  // if (connect(server_sock, (struct sockaddr *)&server_addr,
  //             sizeof(server_addr)) < 0) {
  //   printf("Error while connecting to server\n");
  //   return NULL;
  // }

  struct addrinfo hints, *res;
  int rv, server_sock;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(hostname, port, &hints, &res)) != 0) {
    printf("Error while getting address info\n");
    return NULL;
  }

  printf("IP address: %s\n",
         inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr));

  server_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (server_sock < 0) {
    printf("Error while creating socket\n");
    return NULL;
  }

  if (connect(server_sock, res->ai_addr, res->ai_addrlen) < 0) {
    printf("Error while connecting to server\n");
    return NULL;
  }

  // Send the request to the server:
  if (send(server_sock, "GET / HTTP/1.0\r\n\r\n", 23, 0) < 0) { // TODO: Add
                                                                // support for
                                                                // other methods
    printf("Error while sending request to server\n");
    return NULL;
  }

  // Receive the response from the server:
  char server_response[2000];
  memset(server_response, '\0', sizeof(server_response));
  while ((read_size = recv(server_sock, server_response, 2000, 0)) > 0) {
    // Forward the response from the redirect address back to the client:
    printf("Response from server: %s\n", server_response);
    write(client_sock, server_response, strlen(server_response));
    memset(server_response, '\0', sizeof(server_response));
  }

  if (read_size == 0) {
    printf("Client disconnected\n");
    fflush(stdout);
  } else if (read_size == -1) {
    printf("Error while receiving client message\n");
  }

  // Close the server socket:
  close(server_sock);
  freeaddrinfo(res);

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

    int *arg = malloc(sizeof(int));
    *arg = client_sock;

    // Add the task to the thread pool:
    thread_pool_add_task(pool, handle_client, arg);
  }

  // Closing the socket:
  close(socket_desc);

  return 0;
}