#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>

// The maximum length of an HTTP message line
#define MAX_LINE 256
// The maximum length of an HTTP response message
#define MAX_LENGTH 16*1024
// The size of a chunk of HTTP response to read from the pipe
#define CHUNK_SIZE 1024


void printError(char *);
void printServerError();
void printResponse(char *str);

int debug = 0;


int main(int argc, char **argv) {

    FILE *fp = stdin; // default is to read from stdin

    // Parse command line options.
    int opt;
    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                debug = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [filename]\n", argv[0]);
                exit(1);
        }
    }
    if(optind < argc) {
        if((fp = fopen(argv[optind], "r")) == NULL) {
            perror("fopen");
            exit(1);
        }
    }

    // Read from fp 
    char line[MAX_LINE];
    char *error = fgets(line, MAX_LINE, fp);
    while(error != NULL) {
        // Check if this line is a GET request
        char *p_GET = strtok(line, " ");
        if (strcmp(p_GET, "GET") == 0){
            // Isolating the resource string (which we are interested in)
            char *res_str = strtok(NULL, " ");
            // Isolating the CGI program and query string
            char *cgi_prog = strtok(res_str, "?");
            char *query_str = strtok(NULL, "?");
            // If there is no query string, query_str is set to the empty string
            if (query_str == NULL){
                setenv("QUERY_STRING", "", 1);
            }

            // Setting up pipes and child processes
            int fd1[2];
            if (pipe(fd1) == -1) {
                perror("pipe");
                exit(1);
            }
            int fd2[2];
            if (pipe(fd2) == -1) {
                perror("pipe");
                exit(1);
            }
            int r = fork();
            // Parent process
            if (r > 0){
                /* First we write the required variables to the child,
                so we close the read file descriptor of the first pipe*/
                if (close(fd1[0]) == -1) {
                    perror("close");
                    exit(1);
                }
                if (write(fd1[1], cgi_prog, MAX_LINE) == -1) {
                    if(cgi_prog != NULL){
                        perror("write to pipe");
                    }
                }
                if (write(fd1[1], query_str, MAX_LINE) == -1) {
                    if(query_str != NULL){
                        perror("write to pipe");
                    }
                }
                // We are done writing, so we close write file descriptor of the first pipe
                if (close(fd1[1]) == -1) {
                    perror("close");
                    exit(1);
                }
                
                // We will not be writing using this second pipe
                if (close(fd2[1]) == -1) {
                    perror("close");
                    exit(1);
                }
                // The parent will now be reading output from the child
                dup2(fd2[0], STDIN_FILENO);

                int status;
                if (wait(&status) != -1) {
                    // Child exited abnormally
                    if (WIFSIGNALED(status)) {
                        // Child exited abnormally
                        printServerError();
                        // Close read file descriptor
                        if (close(fd2[0]) == -1) {
                            perror("close");
                            exit(1);
                        }
                    } else {
                        // Read output from child process
                        char output[MAX_LENGTH] = "";
                        // Read chunks, null-terminate them, and append them
                        char chunk[CHUNK_SIZE + 1];
                        int bytes_read = read(fd2[0], chunk, CHUNK_SIZE);
                        chunk[CHUNK_SIZE] = '\0';
                        while(bytes_read > 0) {
                            strcat(output, chunk);
                            bytes_read = read(fd2[0], chunk, CHUNK_SIZE);
                            chunk[CHUNK_SIZE] = '\0';
                        }
                        if (bytes_read == -1) {
                            perror("read from pipe");
                            exit(1);
                        }
                        // Done reading, print output
                        if (bytes_read == 0){
                            // Child exited normally
                            if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)){
                                printResponse(output);
                            } else {
                                char cgi_prog_curr_path[MAX_LINE] = ".";
                                strcat(cgi_prog_curr_path, cgi_prog);
                                printError(cgi_prog_curr_path);
                            }
                        }
                        // Close read file descriptor
                        if (close(fd2[0]) == -1) {
                            perror("close");
                            exit(1);
                        }
                    }
                } else {
                    perror("wait");
                    exit(1);
                }
            // Child Process
            } else if (r == 0){
                /* Child will first read from the parent through the first pipe,
                so we close the write file descriptor*/
                if (close(fd1[1]) == -1) {
                    perror("close");
                    exit(1);
                }
                char cgi_prog_path[MAX_LINE];
                char query_string[MAX_LINE];
                if (read(fd1[0], cgi_prog_path, MAX_LINE) == -1) {
                    perror("read from pipe");
                    exit(1);
                }
                if (read(fd1[0], query_string, MAX_LINE) == -1) {
                    perror("read from pipe");
                }
                setenv("QUERY_STRING", query_string, 1);
                // We are done reading, so we close the read file descriptor of the first pipe
                if (close(fd1[0]) == -1) {
                    perror("close");
                    exit(1);
                }

                // We will not be reading using this second pipe
                if (close(fd2[0]) == -1) {
                    perror("close");
                    exit(1);
                }
                // Output from this child will be redirected to the write file descriptor of the second pipe
                dup2(fd2[1], STDOUT_FILENO);

                // Execute CGI program
                char cgi_prog_curr_path[MAX_LINE] = ".";
                strcat(cgi_prog_curr_path, cgi_prog_path);
                if (execlp(cgi_prog_curr_path, strtok(cgi_prog_path, "/"), NULL) == -1){
                    perror("exec");
                    // Close write file descriptor
                    if (close(fd2[1]) == -1) {
                        perror("close");
                        exit(1);
                    }
                    exit(1);
                }
            // Error in fork
            } else {
                perror("fork");
                exit(1);
            }
        }
        error = fgets(line, MAX_LINE, fp);
    }

    if(fp != stdin) {
        if(fclose(fp) == EOF) {
            perror("fclose");
            exit(1);
        }
    }

    // Occurs if an error occurs or if end of file is reached
    if(error == NULL) {
        perror("fgets");
    }
}


/* Print an http error page  
 * Arguments:
 *    - str is the path to the resource. It does not include the question mark
 * or the query string.
 */
void printError(char *str) {
    printf("HTTP/1.1 404 Not Found\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>404 Not Found</title>\n");
    printf("</head><body>\n");
    printf("<h1>Not Found</h1>\n");
    printf("The requested resource %s was not found on this server.\n", str);
    printf("<hr>\n</body></html>\n");
}


/* Prints an HTTP 500 error page 
 */
void printServerError() {
    printf("HTTP/1.1 500 Internal Server Error\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>500 Internal Server Error</title>\n");
    printf("</head><body>\n");
    printf("<h1>Internal Server Error</h1>\n");
    printf("The server encountered an internal error or\n");
    printf("misconfiguration and was unable to complete your request.<p>\n");
    printf("</body></html>\n");
}


/* Prints a successful response message
 * Arguments:
 *    - str is the output of the CGI program
 */
void printResponse(char *str) {
    printf("HTTP/1.1 200 OK\r\n\r\n");
    printf("%s", str);
}