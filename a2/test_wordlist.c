#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordlist.h"

/* A simple test that calls read_list to build the linked list of words
 * and then prints it to stdout.
 * Try:
 *     test_wordlist examples/small_words5.txt
 *     test_wordlist words5.txt
 * You can redirect the output of this program to a file and use the program
 * "diff" to compare the files. (You might also need to sort the output)
 */

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "   <filename> is a file containing words\n");
        exit(1);
    }

    struct node *dict = read_list(argv[1]);

    struct node *ptr = dict;
    while(ptr != NULL) {
        printf("%s\n", ptr->word);
        ptr = ptr->next;
    }
    return 0;
}
