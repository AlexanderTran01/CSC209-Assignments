#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seqbot_helpers.h"

int main(int argc, char** argv)
{
    if(argc < 3) {
        fprintf(stderr, "usage: seqbot [ <task>  <input> \n");
        fprintf(stderr, "Perform one of the following tasks:\n");
        fprintf(stderr, "    melt <sequence> - compute the melting point of sequence\n");
        fprintf(stderr, "    print <sequence> - print instructions for for sequence\n");
        fprintf(stderr, "    genall <size> - generate and print instructions for all possible molecules of a given size\n");
        fprintf(stderr, "    genfile <file> - print the instructions for each of the sequences in file\n");
        exit(EXIT_FAILURE);
    }
    
    if(strcmp(argv[1], "melt") == 0) {
        char *test_sequence = argv[2];
        int length = strlen(test_sequence);
        int t = calculate_melting_temperature(test_sequence, length);
        if(t == -1) {
            printf("INVALID SEQUENCE, %s", test_sequence);
            return 1;
        } else {
            printf("The melting temperature of %s is %d\n", test_sequence, t); 
        }

    } else if(strcmp(argv[1], "print") == 0) {
        char *test_sequence = argv[2];
        int length = strlen(test_sequence);
        print_instructions(test_sequence, length);

    } else if(strcmp(argv[1], "genall") == 0) {
        int size = atoi(argv[2]);
        generate_all_molecules(size);

    } else if(strcmp(argv[1], "genfile") == 0) {
        generate_molecules_from_file(argv[2]);

    } else {
        fprintf(stderr, "task not recognized: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;

}
