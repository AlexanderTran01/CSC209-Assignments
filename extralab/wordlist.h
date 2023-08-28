#include "common.h"
#define DICT_FILE "words5.txt"

/* Used to store the dictionary list
 */
struct node {
    char word[SIZE];
    struct node *next;
};

struct node *read_list(char *filename);
void print_dictionary(struct node *list);
void free_dictionary(struct node *list);
