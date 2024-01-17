#include "utils.h"
#include <sys/epoll.h>
#define MAX_EVENTS 50

// Create listening socket:
int create_listening_socket(uint16_t _port) {
  int socket_desc;
  struct sockaddr_in server_addr;
  uint16_t port = _port;

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
  server_addr.sin_addr.s_addr = INADDR_ANY;

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
void parse_request(char *buffer, char*method, char *hostname, char *path) { // strtok not thread safe
  char *token = strtok(buffer, " \r\n");
  while (token != NULL) {
    if (strcmp(token, "GET") == 0 || strcmp(token, "POST") == 0) { // TODO: Add support for other methods
      strcpy(method, token);
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

void verify_cache(char* dirpath) {
  // Open directory, count the number of files and delete the oldest one if the number is greater than 10:
  DIR *dir;
  struct dirent *ent;
  int file_count = 0;
  char *oldest_file = NULL;
  time_t oldest_file_time = 0;

  if ((dir = opendir(dirpath)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        file_count++;
        struct stat attr;
        stat(ent->d_name, &attr);
        if (oldest_file == NULL || attr.st_mtime < oldest_file_time) {
          oldest_file = ent->d_name;
          oldest_file_time = attr.st_mtime;
        }
      }
    }
    closedir(dir);

    if (file_count > MAX_CACHE_SIZE) {
      char *file_to_delete = malloc(strlen(dirpath) + strlen(oldest_file) + 1);
      strcpy(file_to_delete, dirpath);
      strcat(file_to_delete, oldest_file);
      remove(file_to_delete);
      free(file_to_delete);
    }

  } else {
    printf("Unable to open directory\n");
    return;
  }
}

void handle_request(ClientRequest* arg){
  printf("Thread #%ld\n", pthread_self()); // Print the thread id;

  //ClientRequest* arg = (ClientRequest*)req;
  int client_sock = arg->socketfd;
  char* client_message = arg->request;

  char* unmodified_message = malloc(2000);
  strcpy(unmodified_message, arg->request); // ----------------- salvam requestul original

  char* filepath = malloc(200);
  char* encodedfilepath; //= malloc(400);
  char* dirpath = malloc(200);
  snprintf(dirpath, 10, "%d", client_sock);
  strcat(dirpath, "/");
    mkdir(dirpath, S_IRWXU);

    // DIR* rootDir = opendir(dirpath);
    // if (rootDir == NULL) {
    //     printf("Unable to open %s directory\n", dirpath);
    //     return;
    // }

  write(STDOUT_FILENO, client_message, strlen(client_message));

  int read_size = strlen(client_message);
  char url[200];
  char hostname[100];
  char method[10];
  char path[200];
  char to_find[2000];
  char first_line[200];

  strcpy(first_line, client_message);
  //  strcpy(to_find, client_message);
  parse_request(client_message, method, hostname, path);

  printf("\n\nMethod: %s\n", method);
  printf("Hostname: %s\n", hostname);
  printf("Path: %s\n", path);

  strcpy(filepath, method);
  strcat(filepath, hostname);
  strcat(filepath, path);

  printf("Filepath: %s\n", filepath);
  encodedfilepath = b64_encode(filepath, strlen(filepath));
  printf("Encoded: %s\n", encodedfilepath);
  char* verify_path = (char*)malloc(strlen(encodedfilepath)+1+strlen(dirpath)*sizeof(char));
  strcpy(verify_path, dirpath);
  strcat(verify_path, encodedfilepath);
  free(encodedfilepath);
  free(filepath);


    if (access(verify_path, F_OK) != 0) { // DACA NU EXISTA, IL CREAM
          // Find the end of the first line
      char *end_of_first_line = strchr(first_line, '\r');
      if (end_of_first_line != NULL) {
        // Null-terminate the first line
        *end_of_first_line = '\0';

    // Add two CRLF sequences to the end of the first line
    strcat(first_line, "\r\n\r\n");

    printf("Modified Request: %s\n", first_line);
     }  else
    printf("Error while modifying request\n");

      // Take the port number from the hostname, if any, else use port 80:
      strcpy(url, hostname);
      // get just the url without the port

      char *port = strstr(url, ":");
      if (port != NULL) {
        *port = '\0';
        port++;
      } else {
        port = "80";
      }

      int int_port = atoi(port);
      printf("Port: %d\n", int_port);

      // struct addrinfo hints, *res;
      // int rv, server_sock; //--------------- Set on what port to get the answer
 
      struct sockaddr_in targetAddr;
      memset(&targetAddr, 0, sizeof(targetAddr));
      targetAddr.sin_family = AF_INET;
      targetAddr.sin_port = htons(int_port); // modify port


    struct hostent* host = gethostbyname(url);
    if (host == NULL) {
      printf("Error while getting host by name\n");
      return NULL;
    }
    memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);

    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);
    connect(targetSocket, (struct sockaddr*)&targetAddr, sizeof(targetAddr));

    printf("Ip address: %s\n", inet_ntoa(targetAddr.sin_addr));
      // Send the request to the server:
      if (write(targetSocket, unmodified_message, strlen(unmodified_message)) < 0) { 
        printf("Error while sending request to server\n");
        return NULL;
      }

      
      int fd = open(verify_path, O_CREAT | O_WRONLY, 0644);
      printf("Writing to file\n");
      int total_length = 0;
      int content_length = 0;
      int offset_content = 0;
      // Receive the response from the server:

      char server_response[4096];
      int flags = fcntl(targetSocket, F_GETFL, 0);
      fcntl(targetSocket, F_SETFL, flags | O_NONBLOCK);
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(targetSocket, &read_fds);

      // Set up timeout struct
      struct timeval timeout;
      timeout.tv_sec = 2;
      timeout.tv_usec = 0;
     while (1) {
    // Use select to wait for data or timeout
      //     timeout.tv_sec = 3;
      // timeout.tv_usec = 0;
    int result = select(targetSocket + 1, &read_fds, NULL, NULL, &timeout);

    if (result > 0) {
        // Data is available to read
        char server_response[4096];
        ssize_t read_size = recv(targetSocket, server_response, sizeof(server_response) - 1, 0);

        if (read_size > 0) {
            server_response[read_size] = '\0';

            // Process the data as needed
            write(fd, server_response, strlen(server_response));
            printf("Read size: %d\n", read_size);
           

            // if (total_length >= content_length + offset_content) {
            //     break;
            // }
        } else if (read_size == 0) {
            // Connection closed
            printf("Connection closed by the server.\n");
            break;
        }
    } else if (result == 0) {
        // Timeout reached
        printf("Timeout reached.\n");
        break;
    } else {
        // Error in select
        perror("Error in select");
        break;
    }
}
      printf("Done writing to file\n");
      close(fd);

      printf("Printing director %s\n", dirpath);
      verify_cache(dirpath); // Delete another entry if needed

      close(targetSocket);

      } 

      // Send the file to the client:
      printf("Sending file to client\n");

      FILE *file = fopen(verify_path, "r");
      if (file == NULL) {
        printf("Unable to open file %s\n", path);
        return -1;
      }

      char buffer[2000];
      size_t bytesRead;
      while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(client_sock, buffer, bytesRead, 0) < 0) {
          printf("Error while sending file content to socket\n");
          break;
        }
        else {
          printf("Send bytes %d \n", bytesRead);
        }
      }
     // send(client_sock, "[FINISHED]",11, 0);
    fclose(file);
  
  return NULL;
}

void handle_disconnect(void* socket_desc){
printf("Thread deconectat #%ld\n", pthread_self()); // Print the thread id;

char dirpath[10];
snprintf(dirpath, 10, "%d", *(int*)socket_desc);
strcat(dirpath, "/");

 //Remove all files from dir and after remove the dir
    DIR *d;
    struct dirent *dir;
    d = opendir(dirpath);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
          if (strcmp(dir->d_name, ".") != 0 || strcmp(dir->d_name, "..") != 0) {
            char* file_to_delete = malloc(strlen(dirpath) + strlen(dir->d_name) + 1);
            strcpy(file_to_delete, dirpath);
            strcat(file_to_delete, dir->d_name);
            remove(file_to_delete);
            free(file_to_delete);
          }
        }
        closedir(d);
    }
    rmdir(dirpath);

  // Close the socket:
  close(*(int*)socket_desc);
  free(socket_desc);
  return NULL;
}

int main(void) {

  int listen_sock, conn_sock, client_size;
  struct sockaddr_in server_addr, client_addr;

  struct epoll_event ev, events[MAX_EVENTS];
  int epollfd, nfds;

  listen_sock = create_listening_socket(8081);
  if (listen_sock < 0) {
    return -1;
  }

      epollfd = epoll_create1(0);
      if (epollfd == -1) {
          perror("epoll_create1");
          exit(EXIT_FAILURE);
      }

      ev.events = EPOLLIN;
      ev.data.fd = listen_sock;
      if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
          perror("epoll_ctl: listen_sock");
          exit(EXIT_FAILURE);
      }

  // Initialize thread pool:
  ThreadPool *pool;
  pool = thread_pool_init(5);

  // Verify if the thread pool was created successfully:
  if (pool == NULL) {
    printf("Error while initializing thread pool\n");
    return -1;
  }

      for (;;) {
          nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
          if (nfds == -1) {
              perror("epoll_wait");
              exit(EXIT_FAILURE);
          }

          for (int n = 0; n < nfds; ++n) {
              if (events[n].data.fd == listen_sock) {
                  conn_sock = accept(listen_sock,
                                     (struct sockaddr *) &client_addr,
                                     &client_size);
                  if (conn_sock == -1) {
                      perror("accept");
                      exit(EXIT_FAILURE);
                  }
                  setnonblocking(conn_sock);
                  ev.events = EPOLLIN | EPOLLET;
                  ev.data.fd = conn_sock;
                  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                                &ev) == -1) {
                      perror("epoll_ctl: conn_sock");
                      exit(EXIT_FAILURE);
                  }
                  printf("Adaugat client %d\n", conn_sock);
              } else {
                  char* buffer = malloc(2000);
                  memset(buffer, '\0', sizeof(buffer));
                  int read_size = recv(events[n].data.fd, buffer, 2000, 0);
                  if (read_size == 0) {
                      printf("Client disconnected\n");
                      int* fdarg = malloc(sizeof(int));
                      *fdarg = events[n].data.fd;
                      thread_pool_add_task(pool, handle_disconnect, fdarg);
                      fflush(stdout);
                  } else if (read_size == -1) {
                      printf("Error while receiving client message\n");
                                            int* fdarg = malloc(sizeof(int));
                       thread_pool_add_task(pool, handle_disconnect, fdarg);
                    
                  } else {
                  ClientRequest* arg = (ClientRequest*)malloc(sizeof(ClientRequest));
                  arg->socketfd = events[n].data.fd;
                  arg->request = buffer;
                  thread_pool_add_task(pool, handle_request, arg); 
                  }

              }
          }
      }
  // Closing the socket:
  close(listen_sock);

  return 0;
}