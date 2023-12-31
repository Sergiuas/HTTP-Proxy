#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];

  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket, we use SOCK_STREAM for TCP
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0) {
    printf("Unable to create socket\n");
    return -1;
  }

  printf("Socket created successfully\n");

  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send connection request to server:
  if (connect(socket_desc, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");

  char buffer[2000];
  memset(buffer, '\0', sizeof(buffer));

  // Read from a file into the buffer:
  FILE *fp = fopen("file_request.txt", "r");
  if (fp == NULL) {
    printf("Error while opening the file\n");
    return -1;
  }

  size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
  fclose(fp);

  // While the coonection is active, talk with the server:
  while (1) {

    printf("\n");
    printf("Enter message: ");
    scanf("%s", client_message);
    // gets(client_message);

    // Send the message to server:
    if (send(socket_desc, buffer, strlen(buffer), 0) < 0) {
      printf("Unable to send message\n");
      return -1;
    }

    // Receive the server's response:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
      printf("Error while receiving server's msg\n");
      return -1;
    }

    printf("Server's response: %s\n", server_message);
  }

  // Close the socket:
  close(socket_desc);

  return 0;
}