#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

void handle_client(int client_socket, const char *target_host, int target_port) {
    char buffer[4096];
    int target_socket;
    ssize_t bytes_received, bytes_sent;

    // Connect to the target server
    target_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_host, &(target_addr.sin_addr));

    if (connect(target_socket, (struct sockaddr*)&target_addr, sizeof(target_addr)) == -1) {
        perror("Connect to target server failed");
        close(client_socket);
        return;
    }

    // Forward data between client and target server
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        bytes_sent = send(target_socket, buffer, bytes_received, 0);
        if (bytes_sent <= 0) {
            perror("Send to target server failed");
            break;
        }

        bytes_received = recv(target_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Receive from target server failed");
            break;
        }

        bytes_sent = send(client_socket, buffer, bytes_received, 0);
        if (bytes_sent <= 0) {
            perror("Send to client failed");
            break;
        }
    }

    // Close sockets
    close(target_socket);
    close(client_socket);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <proxy_port> <target_host> <target_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int proxy_port = atoi(argv[1]);
    const char *target_host = argv[2];
    int target_port = atoi(argv[3]);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(proxy_port);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Proxy server listening on port %d\n", proxy_port);

    // Accept and handle incoming connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Handle client request in a separate function
        handle_client(client_socket, target_host, target_port);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}