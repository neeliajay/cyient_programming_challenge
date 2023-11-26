/**
 * @file client.c
 * @brief TCP client that sends "ping" messages to a server and prints "pong" responses using non-blocking sockets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_IP "127.0.0.1"  /**< IP address of the server */
#define PORT 8080               /**< Default port number for server */
#define PING_MESSAGE "ping"     /**< Message to be sent by the client */

//Function Declaration
int set_nonblocking(int sockfd);

/**
 * @brief Main function to run the client.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit status.
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        if (errno != EINPROGRESS) {
            perror("connection failed");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    // Set the client socket to non-blocking mode
    if (set_nonblocking(sock) == -1) {
        perror("set_nonblocking");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port %d\n", atoi(argv[1]));

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);

    while (1) {
        if (select(sock + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(sock, &read_fds)) {
            char buffer[1024] = {0};

            // Receive the response
            valread = read(sock, buffer, sizeof(buffer));

            if (valread == 0) {
                printf("Server disconnected\n");
                break;
            } else if (valread == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("read error");
                    break;
                }
            } else {
                printf("Received: %s\n", buffer);
            }

            // Send "ping" message
            if (send(sock, PING_MESSAGE, strlen(PING_MESSAGE), 0) == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("send error");
                    break;
                }
            }
            printf("Sent: %s\n", PING_MESSAGE);

            // Reset the set for the next iteration
            FD_ZERO(&read_fds);
            FD_SET(sock, &read_fds);
        }
    }

    close(sock);

    return 0;
}


/**
 * @brief Set a socket to non-blocking mode.
 * @param sockfd Socket file descriptor.
 * @return 0 on success, -1 on failure.
 */
int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}
