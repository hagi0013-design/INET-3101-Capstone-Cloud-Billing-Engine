
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int run_server(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Could not create server socket");
        return -1;
    }

    int option = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &option, sizeof(option));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("Could not bind server");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Could not start listening");
        close(server_fd);
        return -1;
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    int client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0) {
        perror("Could not accept client");
        close(server_fd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer,
                                  sizeof(buffer) - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Client request: %s\n", buffer);

        const char *response = "Request received by billing server.\n";
        size_t length = strlen(response);
        size_t sent = 0;

        while (sent < length) {
            ssize_t bytes_sent = send(client_fd, response + sent,
                                      length - sent, 0);

            if (bytes_sent <= 0) {
                perror("Could not send response");
                break;
            }

            sent += (size_t)bytes_sent;
        }
    } else if (bytes_received < 0) {
        perror("Could not receive client request");
    } else {
        printf("Client disconnected without sending a request.\n");
    }

    close(client_fd);
    close(server_fd);

    return bytes_received > 0 ? 0 : -1;
}

