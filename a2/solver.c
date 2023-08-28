#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "wordle.h"
#include "constraints.h"

/* A global variable to turn on or off extra print statements.
 * See example in solve_subtree
 * 
 * You are welcome to add your own output that is printed if verbose is 1.
 * This output should be help the reader of your program understand it.
 */

int verbose = 0;

int main(int argc, char **argv) {

    FILE *fp = stdin; // default is to read from stdin

    // Parse command line options.
    int opt;
    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
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

    // 1. Get the list of words
    struct node *dict = read_list(DICT_FILE);

    // 2. Read in the wordle input
    struct wordle *w = create_wordle(fp);
    fclose(fp);
    
    // 3. Initialize the root node of the solver tree.  Use init_constraints()
    // to initialize the constraints struct for the root node.
    struct solver_node *word_tree = create_solver_node(NULL, w->grid[0]);
    word_tree->con = init_constraints();
    
    // 4. Build the tree
    solve_subtree(1, w, dict, word_tree);
    
    // 5. Print all the paths in the tree
    char *path[6];
    if(verbose) {
        printf("PRINTING PATHS\n");
    }
    print_paths(word_tree, path, 1, w->num_rows);
    
    // 6. free all dynamically allocated memory
    free_dictionary(dict);
    free_wordle(w);
    free_tree(word_tree);
    
    return 0;
}