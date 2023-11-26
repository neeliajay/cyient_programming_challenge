/**
 * @file server.c
 * @brief TCP server that responds to "ping" messages with "pong" using non-blocking sockets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <Winsock2.h>
//#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080            /**< Default port number for server */
#define MAX_CLIENTS 5        /**< Maximum number of clients the server can handle simultaneously */

volatile sig_atomic_t terminate_flag = 0; /**< Signal flag for graceful termination */

//Function Declarations
void handle_termination_signal(int signo);
void cleanup_and_exit(int server_fd);
int set_nonblocking(int sockfd);

/**
 * @brief Main function to run the server.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit status.
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Set up termination signal handling
    signal(SIGINT, handle_termination_signal);
    signal(SIGTERM, handle_termination_signal);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        cleanup_and_exit(server_fd);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        cleanup_and_exit(server_fd);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen failed");
        cleanup_and_exit(server_fd);
    }

    printf("Server listening on port %d...\n", atoi(argv[1]));

    // Set the server socket to non-blocking mode
    if (set_nonblocking(server_fd) == -1) {
        perror("set_nonblocking");
        cleanup_and_exit(server_fd);
    }

    fd_set read_fds, master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    int max_fd = server_fd;

    while (!terminate_flag) {
        read_fds = master_fds;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            cleanup_and_exit(server_fd);
        }

        for (int fd = 0; fd <= max_fd; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_fd) {
                    // New connection
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1) {
                        perror("accept");
                        continue;
                    }

                    printf("New client connected\n");

                    // Set the new socket to non-blocking mode
                    if (set_nonblocking(new_socket) == -1) {
                        perror("set_nonblocking");
                        close(new_socket);
                        continue;
                    }

                    FD_SET(new_socket, &master_fds);
                    if (new_socket > max_fd) {
                        max_fd = new_socket;
                    }
                } else {
                    // Data from an existing client
                    char buffer[1024] = {0};

                    valread = read(fd, buffer, sizeof(buffer));

                    if (valread == 0) {
                        printf("Client disconnected\n");
                        close(fd);
                        FD_CLR(fd, &master_fds);
                    } else if (valread == -1) {
                        perror("read error");
                        close(fd);
                        FD_CLR(fd, &master_fds);
                    } else {
                        printf("Received from client %d: %s\n", fd, buffer);

                        // Respond with "pong"
                        if (send(fd, "pong", 4, 0) == -1) {
                            perror("send error");
                            close(fd);
                            FD_CLR(fd, &master_fds);
                        }
                        printf("Sent to client %d: pong\n", fd);
                    }
                }
            }
        }
    }

    cleanup_and_exit(server_fd);

    return 0;
}


/**
 * @brief Signal handler to gracefully handle termination signals.
 * @param signo Signal number.
 */
void handle_termination_signal(int signo) {
    terminate_flag = 1;
}

/**
 * @brief Cleanup and exit the server.
 * @param server_fd Server socket file descriptor.
 */
void cleanup_and_exit(int server_fd) {
    close(server_fd);
    exit(EXIT_SUCCESS);
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
