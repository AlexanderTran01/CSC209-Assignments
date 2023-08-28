#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "seqbot_helpers.h"

/* Return the melting temperature of sequence, or -1 if the sequence is invalid.
 * The melting temperature formula is given in the handout.
 * An invalid sequence is a sequence of length 0, or a sequence that contains
 * characters other than 'A', 'C', 'G', 'T'.
 */
int calculate_melting_temperature(char *sequence, int sequence_length) {
    if (sequence_length < 0) {
        return -1;
    }

    int n_A = 0;
    int n_C = 0;
    int n_T = 0;
    int n_G = 0;
    for (int i = 0; i < sequence_length; i++) {
        if (sequence[i] != 'A' && sequence[i] != 'C' && sequence[i] != 'G' && sequence[i] != 'T') {
            return -1;
        }
        switch(sequence[i]) {
            case 'A':
                n_A += 1;
                break;
            case 'C':
                n_C += 1;
                break;
            case 'G':
                n_G += 1;
                break;
            case 'T':
                n_T += 1;
                break;
        }
    }
    int melting_temperature = ((n_A + n_T) * 2 + (n_C + n_G) * 4);
    return melting_temperature;
}

/* Prints the instructions to make a molecule from sequence.
 * If an invalid character is found in sequence print
 * "INVALID SEQUENCE" and return immediately
 */
void print_instructions(char *sequence, int sequence_length) {
    if (sequence_length < 0) {
        printf("INVALID SEQUENCE\n");
        return;
    }

    for (int i = 1; i < sequence_length; i++ ) {
        if (sequence[i] != 'A' && sequence[i] != 'C' && sequence[i] != 'G' && sequence[i] != 'T') {
            printf("INVALID SEQUENCE\n");
            return;
        }
    }

    printf("START\n");


    int streak = 1;
    char previous = sequence[0];

    for (int i = 1; i < sequence_length; i++) {
        if (sequence[i] == previous) {
            streak += 1;
        } else {
            printf("WRITE %c %d\n", previous, streak);
            previous = sequence[i];
            streak = 1;
        }
    }

    if (sequence[sequence_length - 1] == previous) {
        printf("WRITE %c %d\n", previous, streak);
    } else {
        printf("WRITE %c 1\n", sequence[sequence_length - 1]);
    }

    int melting_temperature = calculate_melting_temperature(sequence, sequence_length);
    printf("SET_TEMPERATURE %d\n", melting_temperature);
    printf("END\n");
}


/* Print to standard output all of the sequences of length k.
 * The format of the output is "<length> <sequence> 0" to 
 * correspond to the input format required by generate_molecules_from_file()
 * 
 * Reminder: you are not allowed to use string functions in these files.
 */

//helper
void generate_all_molecules_recurser(char *molecule, int i, int k) {

    if (i == k) {
        printf("%d ", k);
        for (int i = 0; i < k; i++) {
            printf("%c", molecule[i]);
        }
        printf(" 0\n");
        return;
    }

    char bases[] = "ACGT";

    for (int j = 0; j < 4; j++) {
        molecule[i] = bases[j];
        generate_all_molecules_recurser(molecule, i + 1, k);
    }
}

void generate_all_molecules(int k) {
    if (k == 0){
        fprintf(stderr, "error, inputted 0");
        return;
    }
    char molecule[k];
    generate_all_molecules_recurser(molecule, 0, k);
}


/* Print the instructions for each of the sequences found in filename according
 * to the mode provided.
 * filename contains one sequence per line, and the format of each line is
 * "<length> <sequence> <mode>" where 
 *     - <length> is the number of characters in the sequence 
 *     - <sequence> is the array of DNA characters
 *     - <mode> is either 0, 1, 2, or 3 indicating how the <sequence> should 
 *              be modified before printing the instructions. The modes have the 
 *              following meanings:
 *         - 0  - print instructions for sequence (unmodified)
 *         - 1  - print instructions for the the complement of sequence
 *         - 2  - print instructions for the reverse of sequence
 *         - 3  - print instructions for sequence where it is complemented 
 *                and reversed.
 * 
 * Error checking: If any of the following error conditions occur, the function
 * immediately prints "INVALID SEQUENCE" to standard output and exits the 
 * program with a exit code of 1.
 *  - length does not match the number of characters in sequence
 *  - length is not a positive number
 *  - sequence contains at least one invalid character
 *  - mode is not a number between 0 and 3 inclusive
 * 
 * You do not need to verify that length or mode are actually integer numbers,
 * only that they are in the correct range. It is recommended that you use a 
 * fscanf to read the numbers and fgetc to read the sequence characters.
 */

//helpers
char *reverse_array(char *array, int s) {
    char *reversed = malloc(sizeof(char) * s);
    for (int i = 0; i < s; i++) {
        reversed[i] = array[s - 1 - i];
    }
    return reversed;
}

char *take_complement(char *array, int s) {
    char *complements = malloc(sizeof(char) * s);
    for (int i = 0; i < s; i++) {
        switch(array[i]) {
            case 'A':
                complements[i] = 'T';
                break;
            case 'C':
                complements[i] = 'G';
                break;
            case 'G':
                complements[i] = 'C';
                break;
            case 'T':
                complements[i] = 'A';
                break;
        }
    }
    return complements;
}

void generate_molecules_from_file(char* filename) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "fopen failed\n");
        return;
    }

    int length;
    int mode;
    char space_reader[2];

    while (fscanf(file, "%d", &length) == 1) {
        if (length < 0){
            printf("INVALID SEQUENCE\n");
            exit(1);
        }

        fgets(space_reader, 2, file);
        if (space_reader[0] != ' ') {
            printf("INVALID SEQUENCE\n");
            exit(1);
        }

        char sequence[length + 1];

        if (fgets(sequence, length + 1, file) == NULL){
            fprintf(stderr, "fgets failed\n");
            return;
        }

        for (int i = 0; i < length; i++ ) {
            if (sequence[i] != 'A' && sequence[i] != 'C' && sequence[i] != 'G' && sequence[i] != 'T') {
                printf("INVALID SEQUENCE\n");
                exit(1);
            }
        }

        fgets(space_reader, 2, file);
        if (space_reader[0] != ' ') {
            printf("INVALID SEQUENCE\n");
            exit(1);
        }
        
        if (fscanf(file, "%d", &mode) != 1){
            printf("INVALID SEQUENCE\n");
            exit(1);
        }
        if (mode > 3 || mode < 0) {
            printf("INVALID SEQUENCE\n");
            exit(1);
        }
        char *complements = take_complement(sequence, length);
        char *reversed = reverse_array(sequence, length);
        char *both = reverse_array(complements, length);
        switch(mode) {
            case 0: ;
                print_instructions(sequence, length);
                break;
            case 1: ;
                print_instructions(complements, length);
                break;
            case 2: ;
                print_instructions(reversed, length);
                break;
            case 3: ;
                print_instructions(both, length);
                break;
        }
        free(complements);
        free(reversed);
        free(both);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "fclose failed\n");
        return;
    }
}