#include "utils.h"
#include <sys/epoll.h>
#include <signal.h>
#include <sys/signalfd.h>
#define MAX_EVENTS 50

blackList *blacklist_head;

int create_listening_socket(uint16_t _port) {
  int socket_desc;
  struct sockaddr_in server_addr;
  uint16_t port = _port;

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0) {
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  int setopt = 1;

  if (-1 == setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&setopt,
                       sizeof(setopt))) {
    error("ERROR setting socket options");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  if (listen(socket_desc, 5) < 0) {
    printf("Error while listening\n");
    return -1;
  }
  printf("Listening for incoming connections.....\n");

  return socket_desc;
}

void parse_request(char *buffer, char*method, char *hostname, char *path) { 
  char *token = strtok(buffer, " \r\n");
  while (token != NULL) {
    if (strcmp(token, "GET") == 0 || strcmp(token, "POST") == 0) { 
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
        if (oldest_file == NULL || attr.st_atime < oldest_file_time) {
          oldest_file = ent->d_name;
          oldest_file_time = attr.st_atime;
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
  printf("Thread #%ld\n", pthread_self()); 

  int client_sock = arg->socketfd;
  char* client_message = arg->request;

  char* unmodified_message = malloc(2000);
  strcpy(unmodified_message, arg->request); 

  char* filepath = malloc(200);
  char* encodedfilepath;
  char* dirpath = malloc(200);
  snprintf(dirpath, 10, "%d", client_sock);
  strcat(dirpath, "/");
  mkdir(dirpath, S_IRWXU);

  write(STDOUT_FILENO, client_message, strlen(client_message));

  int read_size = strlen(client_message);
  char url[200];
  char hostname[100];
  char method[10];
  char path[200];
  char to_find[2000];
  char first_line[200];

  strcpy(first_line, client_message);
  parse_request(client_message, method, hostname, path);

  printf("\n\nMethod: %s\n", method);
  printf("Hostname: %s\n", hostname);
  printf("Path: %s\n", path);

  if (verify_hostname(hostname, blacklist_head) == true) {
    pthread_exit(NULL);
    return NULL;
  }

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
      char *end_of_first_line = strchr(first_line, '\r');
      if (end_of_first_line != NULL) {
        *end_of_first_line = '\0';
    strcat(first_line, "\r\n\r\n");

    printf("Modified Request: %s\n", first_line);
     }  else
    printf("Error while modifying request\n");

      strcpy(url, hostname);

      char *port = strstr(url, ":");
      if (port != NULL) {
        *port = '\0';
        port++;
      } else {
        port = "80";
      }

      int int_port = atoi(port);
      printf("Port: %d\n", int_port);
 
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
      if (write(targetSocket, unmodified_message, strlen(unmodified_message)) < 0) { 
        printf("Error while sending request to server\n");
        return NULL;
      }

      write_log(unmodified_message);
      
      //int fd = open(verify_path, O_CREAT | O_WRONLY, 0644);
      FILE* wfile = fopen(verify_path, "wb");
      printf("Writing to file\n");
      char server_response[4096];
      int flags = fcntl(targetSocket, F_GETFL, 0);
      fcntl(targetSocket, F_SETFL, flags | O_NONBLOCK);
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(targetSocket, &read_fds);

      struct timeval timeout;
      timeout.tv_sec =0;
      timeout.tv_usec = 500000;

     while (1) {
          int result = select(targetSocket + 1, &read_fds, NULL, NULL, &timeout);

          if (result > 0) {
              char server_response[4097];
              ssize_t read_size = recv(targetSocket, server_response, BUFFER_SIZE - 1, 0);

              if (read_size > 0) {
                  server_response[read_size] = '\0';
                  ssize_t writed_bytes = 0;
                  ssize_t total_saved = read_size;
                  while ((writed_bytes = fwrite(server_response+writed_bytes, 1, total_saved, wfile)) > 0 && total_saved > 0) {
                      total_saved -= writed_bytes;
                   printf("Read size: %d\n", read_size);
                   printf("Writed bytes: %d\n", writed_bytes);
                  }
            
              } else if (read_size == 0) {
                  printf("Connection closed by the server.\n");
                  break;
              }
          } else if (result == 0) {
              printf("Timeout reached.\n");
              break;
          } else {
              perror("Error in select");
              break;
          }
      }
            printf("Done writing to file\n");
              fclose(wfile);

          struct stat st;
          stat(verify_path, &st);
          long file_size = st.st_size;
          if (file_size == 0) {
            printf("File is empty - delete entry\n");
            remove(verify_path);
          }



            printf("Printing director %s\n", dirpath);
            verify_cache(dirpath); 

            close(targetSocket);

      } 


      printf("Sending file to client\n");

      FILE *file = fopen(verify_path, "rb");
      if (file == NULL) {
        printf("Unable to open file %s\n", verify_path);
        pthread_exit(NULL);
        return -1;
      }

      verify_cache(dirpath);  
       
      fseek(file, 0, SEEK_END);
      long file_size = ftell(file);
      rewind(file);
      char* buffer;
      buffer = (char*)malloc(sizeof(char) * file_size);
      if (buffer == NULL) {
        printf("Error allocating memory\n");
        return -1;
      }

      size_t bytesRead;
      bytesRead = fread(buffer, 1, file_size, file);


        if (send(client_sock, buffer, bytesRead, 0) < 0) {
          printf("Error while sending file content to socket\n");
        }
        else {
          printf("Send bytes %d \n", bytesRead);
        }
        
    fclose(file);
    
  return NULL;
}

void handle_disconnect(void* socket_desc){
printf("Thread deconectat #%ld\n", pthread_self());

char dirpath[10];
snprintf(dirpath, 10, "%d", *(int*)socket_desc);
strcat(dirpath, "/");

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

  close(*(int*)socket_desc);
  free(socket_desc);
  return NULL;
}

int main(void) {

  initialize_blackList(&blacklist_head);

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

  ThreadPool *pool;
  pool = thread_pool_init(10);
  int sockets_vector[50];
  for (int i = 0; i < 50; i++) {
    sockets_vector[i] = 0;
  }

  if (pool == NULL) {
    printf("Error while initializing thread pool\n");
    return -1;
  }

  ev.events = EPOLLIN;
  ev.data.fd = STDIN_FILENO;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
      perror("epoll_ctl: stdin");
      exit(EXIT_FAILURE);
  }


    int sfd, retsfd;
    sigset_t sigset;

    retsfd = sigprocmask(SIG_SETMASK, NULL, &sigset);
    if (retsfd == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);

    sfd = signalfd(-1, &sigset, 0);
    if (sfd == -1) {
        perror("signalfd");
        exit(EXIT_FAILURE);
    }

      ev.events = EPOLLIN;
      ev.data.fd = sfd;
      if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
          perror("epoll_ctl: sfd");
          exit(EXIT_FAILURE);
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
                  sockets_vector[conn_sock] = 1;
              } else if  (events[n].data.fd == sfd)
              {
                printf("Received signal\n");
                struct signalfd_siginfo fdsi;
                ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
                if (s != sizeof(struct signalfd_siginfo)) {
                    perror("read(signalfd)");
                    exit(EXIT_FAILURE);
                }

                if (fdsi.ssi_signo == SIGINT) {
                    printf("Received SIGINT. Cleaning up and exiting.\n");
                    exit(EXIT_SUCCESS);
                }
              }
              else {
                  char* buffer = malloc(2000);
                  memset(buffer, '\0', sizeof(buffer));
                  int read_size = recv(events[n].data.fd, buffer, 2000, 0);
                  if (read_size == 0) {
                      printf("Client disconnected\n");
                      int* fdarg = malloc(sizeof(int));
                      *fdarg = events[n].data.fd;
                      thread_pool_add_task(pool, handle_disconnect, fdarg);
                      sockets_vector[events[n].data.fd] = 0;
                      fflush(stdout);
                  } else if (read_size == -1) {
                      printf("Error while receiving client message\n");
                      int* fdarg = malloc(sizeof(int));
                      *fdarg = events[n].data.fd;
                       thread_pool_add_task(pool, handle_disconnect, fdarg);
                       sockets_vector[events[n].data.fd] = 0;
                    
                  } else {
                  

                  ClientRequest* arg = (ClientRequest*)malloc(sizeof(ClientRequest));
                  arg->socketfd = events[n].data.fd;
                  arg->request = buffer;
                  thread_pool_add_task(pool, handle_request, arg); 
                  }

              }
          }
      }

  for (int i=0; i<50; i++){
    if (sockets_vector[i] == 1){
            int* fdarg = malloc(sizeof(int));
      *fdarg = i;
        thread_pool_add_task(pool, handle_disconnect, fdarg);
        sockets_vector[i] = 0;
    }
  }
  close(listen_sock);

  return 0;
}