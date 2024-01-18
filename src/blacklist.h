#ifndef BLACKLIST_H
#define BLACKLIST_H
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
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <utime.h>


typedef struct blackList {
    char* hostname;
    struct blackList* next;
} blackList;

pthread_mutex_t blacklist_mutex; 
pthread_mutex_t log_mutex;

void write_log(char* log) { 
  pthread_mutex_lock(&log_mutex);


  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  char log_message[1000];
  sprintf(log_message, "[%02d:%02d:%02d] %s", timeinfo->tm_hour,
          timeinfo->tm_min, timeinfo->tm_sec, log);

  FILE* file = fopen("log.txt", "a");
  if (file == NULL) {
    printf("Error opening file\n");
    return;
  }
  fprintf(file, "%s\n\n", log_message);

  fclose(file);
  pthread_mutex_unlock(&log_mutex);

}

void initialize_blackList(blackList** head) {
    *head = NULL;

    FILE* file = fopen("blacklist.txt", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        line[read - 1] = '\0';

        add_blackList(head, line);
    }

    if (pthread_mutex_init(&blacklist_mutex, NULL) != 0) {
    printf("Error while initializing mutex\n");
    return -1;
  }
    fclose(file);
}

void add_blackList(blackList** head, char* hostname) {
    blackList* new_node = (blackList*)malloc(sizeof(blackList));
    if (new_node == NULL) {
        printf("Error allocating memory\n");
        return;
    }

    printf("Adding %s to blacklist\n", hostname);
    new_node->hostname = (char*)malloc(strlen(hostname) + 1);
    if (new_node->hostname == NULL) {
        printf("Error allocating memory\n");
        return;
    }

    strcpy(new_node->hostname, hostname);

    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    blackList* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = new_node;
}


bool verify_hostname(char* hostname, blackList* head) {
  pthread_mutex_lock(&blacklist_mutex); 

  blackList* current = head;
  while (current != NULL) {
    if (strcmp(current->hostname, hostname) == 0) {
      printf("Hostname %s is blacklisted\n", hostname);
      return true;
      pthread_mutex_unlock(&blacklist_mutex);
    }

    current = current->next;
  }

  pthread_mutex_unlock(&blacklist_mutex);
  return false;
}


#endif // BLACKLIST_H
