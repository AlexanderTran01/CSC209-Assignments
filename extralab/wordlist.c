#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordlist.h"


/* Read the words from a filename and return a linked list of the words.
 *   - The newline character at the end of the line must be removed from
 *     the word stored in the node.
 *   - You an assume you are working with Linux line endings ("\n").  You are
 *     welcome to also handle Window line endings ("\r\n"), but you are not
 *     required to, and we will test your code on files with Linux line endings.
 *   - The time complexity of adding one word to this list must be O(1)
 *     which means the linked list will have the words in reverse order
 *     compared to the order of the words in the file.
 *   - Do proper error checking of fopen, fclose, fgets
 */

struct node *create_node(char *word, struct node *next) {
    struct node *new_node = malloc(sizeof(struct node));
    strncpy(new_node->word, word, SIZE);
    new_node->next = next;
    return new_node;
}

struct node *read_list(char *filename) {

    //Open file and error check
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "fopen failed\n");
        exit(1);
    }

    /*Read each line from the file, create a node with the line's word as the node's value,
     *and insert it to the linked list
     */
    char line[SIZE];
    struct node *front = NULL;
    while (fgets(line, SIZE + 1, file) != NULL) {
        line[WORDLEN] = '\0';
        front = create_node(line, front);
    }

    //Close file and error check
    if (fclose(file) != 0) {
        fprintf(stderr, "fclose failed\n");
        exit(1);
    }
    return front;
}

/* Print the words in the linked-list list one per line
 */
void print_dictionary(struct node *list) {
    struct node *curr = list;
    while (curr != NULL) {
        printf("%s\n", curr->word);
        curr = curr->next;
    }
}

/* Free all of the dynamically allocated memory in the dictionary list 
 */
void free_dictionary(struct node *list) {
    struct node *curr = list;
    while (curr != NULL) {
        printf("%s\n", curr->word);
        struct node *temp = curr->next;
        free(curr);
        curr = temp;
    }
}

