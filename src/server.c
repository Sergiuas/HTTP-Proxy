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
void parse_request(char *buffer, char*method, char *hostname, char *path) {
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
  char* filepath = malloc(200);
  char* encodedfilepath; //= malloc(400);
  char* dirpath = malloc(200);
  snprintf(dirpath, 10, "%d", client_sock);
  strcat(dirpath, "/");

    mkdir(dirpath, S_IRWXU);

    DIR* rootDir = opendir(dirpath);
    if (rootDir == NULL) {
        printf("Unable to open %s directory\n", dirpath);
        return;
    }

  write(STDOUT_FILENO, client_message, strlen(client_message));

  int read_size = strlen(client_message);
  char hostname[100];
  char method[10];
  char path[200];
  char to_find[2000];
  char first_line[200];

  strcpy(first_line, client_message);
  //  strcpy(to_find, client_message);
  parse_request(client_message, method, hostname, path);

  printf("\n\nnMethod: %s\n", method);
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
  free(dirpath);
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
      strcpy(to_find, hostname);
      char *port = strstr(to_find, ":");
      if (port != NULL) {
        *port = '\0';
        port++;
      } else {
        port = "80";
      }

      printf("Port: %s\n", port);

      struct addrinfo hints, *res;
      int rv, server_sock; //--------------- Set on what port to get the answer
 
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


      int fd = open(verify_path, O_CREAT | O_WRONLY, 0644);
    printf("Writing to file\n");
      // Receive the response from the server:
      char server_response[2000];
      memset(server_response, '\0', sizeof(server_response));
      while ((read_size = recv(server_sock, server_response, 2000, 0)) > 0) {
        // Forward the response from the redirect address back to the client:
        // printf("Response from server: %s\n", server_response);
        write(fd, server_response, strlen(server_response));
        memset(server_response, '\0', sizeof(server_response));
      }
      close(fd);
      close(server_sock);

      verify_cache(dirpath); // Delete another entry if needed
    // code repetitiv --- ar trb sa l pun mai jos; -------
        fd = open(verify_path, O_RDONLY);
        if (fd == -1){
            printf("Unable to open file %s\n", path);
            return -1;
        }

    struct stat file_stat;
    int file_size;
    int len;

    if (fstat(fd, &file_stat) < 0){
        printf("Unable to get file stats for %s\n", path);
        return -1;
    }

    file_size = file_stat.st_size;

    printf("Sending file size %d\n", file_size);

    if ((len=send(client_sock, &file_size, sizeof(file_size), 0)) < 0){
        printf("Unable to send file size for %s\n", path);
        return -1;
    } else if (len == 0){
        printf("Len is 0\n");
        return 0;
    }
    
    // TRIMIT FISIERUL la qt client ------------------------------

    if (sent_bytes < 0){
        printf("Unable to send file %s\n", path);
        return -1;
    }

    //printf("Bytes sent: %d\n", sent_bytes);

    close(fd);

      // Close the server socket:
      close(server_sock);
      freeaddrinfo(res);
      } 
      else {    
          printf("Sending already existing file\n");
          // DACA EXISTA, facem sendfile
         int fd = open(verify_path, O_RDONLY);
          if (fd == -1){
            printf("Unable to open file %s\n", verify_path);
            return -1;
          }

        struct stat file_stat;
        int file_size;
        int len;

        if (fstat(fd, &file_stat) < 0){
            printf("Unable to get file stats for %s\n", verify_path);
            return -1;
        }

        file_size = file_stat.st_size;

        printf("Sending file size %d\n", file_size);

        if ((len=send(client_sock, &file_size, sizeof(file_size), 0)) < 0){
            printf("Unable to send file size for %s\n", path);
            return -1;
        } else if (len == 0){
            printf("Len is 0\n");
            return 0;
        }
        

        int remain_data = file_size;
        off_t offset=0;
        int sent_bytes = 0;

        while(((sent_bytes = sendfile(client_sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)){
            remain_data -= sent_bytes;
        } 


        if (sent_bytes < 0){
            printf("Unable to send file %s\n", path);
            return -1;
        }

        close(fd);

      }

  // Close the socket:
  close(client_sock);
  
  return NULL;
}

void handle_disconnect(void* socket_desc){
printf("Thread #%ld\n", pthread_self()); // Print the thread id;
}

int main(void) {

  int listen_sock, conn_sock, client_size;
  struct sockaddr_in server_addr, client_addr;

  struct epoll_event ev, events[MAX_EVENTS];
  int epollfd, nfds;

  listen_sock = create_listening_socket(8080);
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