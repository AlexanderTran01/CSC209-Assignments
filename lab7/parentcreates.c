#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    int i;
    int iterations;

    if (argc != 2) {
        fprintf(stderr, "Usage: forkloop <iterations>\n");
        exit(1);
    }

    iterations = strtol(argv[1], NULL, 10);

    int parent = getpid();
    for (i = 0; i < iterations; i++) {
        if (getpid() == parent){
            int n = fork();
            if (n < 0) {
            perror("fork");
            exit(1);
            }
        }
        printf("ppid = %d, pid = %d, i = %d\n", getppid(), getpid(), i);
    }

    for (i = 0; i < iterations; i++) {
        int status;
        wait(&status);
    }
    
    return 0;
}
