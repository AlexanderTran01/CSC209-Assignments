#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <sys/time.h>

#include "wrapsock.h"
#include "ws_helpers.h"

#define MAXCLIENTS 10

int handleClient(struct clientstate *cs, char *line);
int accept_connection(int fd, struct clientstate *clients);
int read_from(int client_index, struct clientstate *clients);

// You may want to use this function for initial testing
//void write_page(int fd);

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: wserver <port>\n");
        exit(1);
    }
    unsigned short port = (unsigned short)atoi(argv[1]);
    int listenfd;
    struct clientstate clients[MAXCLIENTS];


    // Set up the socket to which the clients will connect
    listenfd = setupServerSocket(port);

    initClients(clients, MAXCLIENTS);

    // The client accept - message accept loop. First, we prepare to listen to multiple
    // file descriptors by initializing a set of file descriptors.
    int max_fd = listenfd;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(listenfd, &read_fds);

    // Create timeout so that Select times out and exits with value 0 if it has been idle for 5 minutes.
    struct timeval timeout;
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    int connections_handled = 0;
    // We choose to handle 10 connections and then terminate
    while (connections_handled <= 10) {
        // select updates the fd_set it receives, so we always use a copy and retain the original.
        fd_set select_fds = read_fds;
        Select(max_fd + 1, &select_fds, NULL, NULL, &timeout);

        // Is it the original socket? Create a new connection ...
        if (FD_ISSET(listenfd, &select_fds)) {
            int client_fd = accept_connection(listenfd, clients);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &read_fds);
            fprintf(stderr, "Accepted connection\n");
            connections_handled += 1;
            if (connections_handled > 10){
                fprintf(stderr, "server: max connections\n");
                exit(0);
            }
        }

        // Check the clients.
        for (int index = 0; index < MAXCLIENTS; index++) {
            // Check if this client slot currently has a client ready to read from
            if (clients[index].sock > -1 && FD_ISSET(clients[index].sock, &select_fds)) {
                FD_SET(clients[index].fd[0], &read_fds);
                int client_sock = clients[index].sock;
                char buf[MAXLINE + 1] = {'\0'};
                // Read entire request (while loop repeats if there is more to read)
                while (read(client_sock, buf, MAXLINE) > 0) {
                    buf[MAXLINE] = '\0';
                    int handled = handleClient(&clients[index], buf);
                    // Request is not a GET request or is poorly formatted, close socket and clean up resources
                    if (handled == -1){
                        Close(client_sock);
                        FD_CLR(client_sock, &read_fds);
                        resetClient(&clients[index]);
                    } // Request is ok to process
                    else if (handled == 1) {
                        // Pipe descriptor to read CGI output from
                        int read_fd = processRequest(&clients[index]);
                        // processRequest has handled the error message, no need to continue
                        if (read_fd == -1){
                            // Clean up resources
                            Close(client_sock);
                            FD_CLR(client_sock, &read_fds);
                            resetClient(&clients[index]);
                        // Request processed successfully, move forward in reading output
                        } else {
                            
                            int bytes;
                            // Read all output
                            while ((bytes = read(read_fd, clients[index].optr, MAXPAGE)) != 0) {
                                if (bytes == -1) {
                                    perror("read from pipe");
                                    printServerError(client_sock);
                                    exit(1);
                                }
                                else {
                                    clients[index].optr += bytes;
                                }
                            }
                            // Read has returned 0, call wait
                            int status;
                            if (wait(&status) != -1) {
                                // Child exited abnormally
                                if (WIFSIGNALED(status)) {
                                    // Child exited abnormally
                                    printServerError(client_sock);
                                    // Clean up resources
                                    Close(clients[index].fd[0]);
                                    Close(client_sock);
                                    FD_CLR(client_sock, &read_fds);
                                    resetClient(&clients[index]);
                                }
                                // Child exited normally
                                else if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)){
                                    printOK(client_sock, clients[index].output, (clients[index].optr) - (clients[index].output));
                                    Close(clients[index].fd[0]);
                                    Close(client_sock);
                                    FD_CLR(client_sock, &read_fds);
                                    resetClient(&clients[index]);
                                } else {
                                    printServerError(client_sock);
                                    // Clean up resources
                                    Close(clients[index].fd[0]);
                                    Close(client_sock);
                                    FD_CLR(client_sock, &read_fds);
                                    resetClient(&clients[index]);
                                }
                            } else {
                                perror("wait");
                                printServerError(client_sock);
                                exit(1);
                            }
                        }
                    } else {
                        fprintf(stderr, "Request is incomplete, waiting...\n");
                    }
                }
            }
        }
    }
    return 0;
}

/* Update the client state cs with the request input in line.
 * Intializes cs->request if this is the first read call from the socket.
 * Note that line must be null-terminated string.
 *
 * Return 0 if the get request message is not complete and we need to wait for
 *     more data
 * Return -1 if there is an error and the socket should be closed
 *     - Request is not a GET request
 *     - The first line of the GET request is poorly formatted (getPath, getQuery)
 * 
 * Return 1 if the get request message is complete and ready for processing
 *     cs->request will hold the complete request
 *     cs->path will hold the executable path for the CGI program
 *     cs->query_string will hold the query string
 *     cs->output will be allocated to hold the output of the CGI program
 *     cs->optr will point to the beginning of cs->output
 */
int handleClient(struct clientstate *cs, char *line) {
    // Request is NULL if this is the first read call from socket, so we allocate space for it
    if(cs->request == NULL) {
        cs->request = malloc(MAXLINE + 1);
        cs->request[0] = '\0';
    }

    strcat(cs->request, line);
    // An HTTP request ends with a blank line in the form of "\r\n\r\n"
    // if "\r\n\r\n" is not present, the request message is not complete
    if(strstr(cs->request, "\r\n\r\n") == NULL) {
        return 0;
    }

    // Updating cs's fields
    char *path = getPath(cs->request);
    if (path == NULL) {
        return -1;
    } else {
        cs->path = malloc(sizeof(path));
        cs->path[0] = '\0';
        strcpy(cs->path, path);
    }
    // If the resource is favicon.ico we will ignore the request
    if(strcmp("favicon.ico", cs->path) == 0){
        // A suggestion for debugging output
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s (ignoring)\n", cs->path);
		printNotFound(cs->sock);
        return -1;
    }
    char *query = getQuery(cs->request);
    if (query == NULL) {
        return -1;
    } else {
        cs->query_string = malloc(sizeof(query));
        cs->query_string[0] = '\0';
        strcpy(cs->query_string, query);
    }
    cs->output = malloc(MAXPAGE);
    cs->optr = cs->output;

    // A suggestion for printing some information about each client. 
    // You are welcome to modify or remove these print statements
    fprintf(stderr, "Client: sock = %d\n", cs->sock);
    fprintf(stderr, "        path = %s\n", cs->path);
    fprintf(stderr, "        query_string = %s\n", cs->query_string);

    return 1;
}

/*
 * Accept a connection. Note that a new file descriptor is created for
 * communication with the client. The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor.
 */
int accept_connection(int fd, struct clientstate *clients) {
    int client_index = 0;
    while (client_index < MAXCLIENTS && clients[client_index].sock != -1) {
        client_index++;
    }

    if (client_index == MAXCLIENTS) {
        fprintf(stderr, "server: max connections\n");
        exit(0);
    }

    int client_fd = Accept(fd, NULL, NULL);

    clients[client_index].sock = client_fd;
    return client_fd;
}