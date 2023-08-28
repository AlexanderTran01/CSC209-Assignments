#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constraints.h"

/* Read an input file that specifies a sequence of operations on
 * a constraints struct and apply the operations.  Print the resulting
 * constraints struct.
 * 
 * Assume that the input file is correctly structured and the input is valid.
 * 
 * A line of input begins with a character that indicates the function to call.
 * The remainder of the line is a space separated list of parameters to the
 * function.
 * 
 * Each of the helper functions parse an input line from the file and call
 * the appropriate function to set constraints in the struct.
 */

void do_green(char *line, struct constraints *con) {
    // start after the 'g' and extract the arguments for set_green
    char *cstr = strtok(&line[2], " "); 
    int index = atoi(strtok(NULL, " "));
    set_green(cstr[0], index, con);
}

void do_yellow(char *line, struct constraints *con) {
    // extract each argument for set_yellow
    int index = atoi(strtok(&line[2], " "));
    char *cur_tiles = strtok(NULL, " ");
    char *next_tiles = strtok(NULL, " ");
    char *word = strtok(NULL, " "); 
    set_yellow(index, cur_tiles, next_tiles, word, con);
}

void do_cannot_be(char *line, struct constraints *con){
    char *word = strtok(&line[2], " "); 
    add_to_cannot_be(word, con);
}


int main(int argc, char **argv) {
    if(argc != 2){
        fprintf(stderr, "Usage: test_constraints <inputfile>\n");
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        fprintf(stderr, "Could not open %s\n", argv[1]);
        exit(1);
    }
    char line[MAXLINE];

    struct constraints *con = init_constraints();

    while(fgets(line, MAXLINE, fp) != NULL) {
        // remove newline character
        if(line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1 ] = '\0';
            if(line[strlen(line) - 1] == '\r') {
                line[strlen(line) - 1 ] = '\0';
            }
        }
        if(line[0] == 'g') {
            do_green(line, con);
        } else if(line[0] == 'y') {
            do_yellow(line, con);
        } else if(line[0] == 'c') {
            do_cannot_be(line, con);
        }
    }

    print_constraints(con);
    return 0;
}