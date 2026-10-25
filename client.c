
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024

int run_client(void)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Could not create client socket");
        return -1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS,
                  &server_address.sin_addr) != 1) {
        fprintf(stderr, "Invalid server address.\n");
        close(socket_fd);
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0) {
        perror("Could not connect to server");
        close(socket_fd);
        return -1;
    }

    const char *request = "Hello from the billing client.\n";
    size_t request_length = strlen(request);
    size_t sent = 0;

    while (sent < request_length) {
        ssize_t bytes_sent = send(socket_fd, request + sent,
                                  request_length - sent, 0);

        if (bytes_sent <= 0) {
            perror("Could not send request");
            close(socket_fd);
            return -1;
        }

        sent += (size_t)bytes_sent;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(socket_fd, buffer,
                                  sizeof(buffer) - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Server response: %s", buffer);
    } else if (bytes_received == 0) {
        fprintf(stderr, "Server closed the connection.\n");
        close(socket_fd);
        return -1;
    } else {
        perror("Could not receive server response");
        close(socket_fd);
        return -1;
    }

    close(socket_fd);
    return 0;
}

