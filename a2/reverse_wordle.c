#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wordle.h"
#include "constraints.h"

/* Read the wordle grid and solution from fp. 
 * Return a pointer to a wordle struct.
 * See sample files for the format. Assume the input file has the correct
 * format.  In other words, the word on each is the correct length, the 
 * words are lower-case letters, and the line ending is either '\n' (Linux,
 * Mac, WSL) or '\r\n' (Windows)
 */
struct wordle *create_wordle(FILE *fp) {
    struct wordle *w = malloc(sizeof(struct wordle));
    char line[MAXLINE]; 
    w->num_rows = 0;

    while(fgets(line, MAXLINE, fp ) != NULL) {
        
        // remove the newline character(s) 
        char *ptr;
        if(((ptr = strchr(line, '\r')) != NULL) ||
           ((ptr = strchr(line, '\n')) != NULL)) {
            *ptr = '\0';
        }
        
        strncpy(w->grid[w->num_rows], line, SIZE);
        w->grid[w->num_rows][SIZE - 1] = '\0';
        w->num_rows++;
    }
    return w;
}


/* Create a solver_node and return it.
 * If con is not NULL, copy con into dynamically allocated space in the struct
 * If con is NULL set the new solver_node con field to NULL.
 * Tip: struct assignment makes copying con a one-line statement
 */
struct solver_node *create_solver_node(struct constraints *con, char *word) {

    struct solver_node *new_node = malloc(sizeof(struct solver_node));

    if (con != NULL){
        new_node->con = malloc(sizeof(struct constraints));
        *(new_node->con) = *con;
    }
    if (con == NULL){
        new_node->con = NULL;
    }

    strncpy(new_node->word, word, SIZE);

    new_node->next_sibling = NULL;
    new_node->child_list = NULL;

    return new_node;
}

/* Return 1 if "word" matches the constraints in "con" for the wordle "w".
 * Return 0 if it does not match
 */
int match_constraints(char *word, struct constraints *con, struct wordle *w, int row) {

    for (int i = 0; i < WORDLEN; i++){

        /*if word[index] is not in the must_be[index] string, 
         *then this word does not match the constraints.*/
        if (strlen(con->must_be[i]) != 0){
            if (strchr(con->must_be[i], word[i]) == NULL){
                return 0;
        }
        }

        /*If must_be[index] is the empty string and word[index] is in the cannot_be set,
         *this word does not match the constraints.*/
        if (strlen(con->must_be[i]) == 0 && con->cannot_be[word[i] - 'a'] == 1){
            return 0;
        }

        //Check all yellow tiles for duplicates of letters in the solution
        if (w->grid[row][i] == 'y'){
            for (int j = i + 1; j < WORDLEN; j++){
                if (word[j] == w->grid[row][i]){
                    return 0;
                }
            }
        }
    }

    return 1;
}
/* remove "letter" from "word"
 * "word" remains the same if "letter" is not in "word"
 */
void remove_char(char *word, char letter) {
    char *ptr = strchr(word, letter);
    if(ptr != NULL) {
        *ptr = word[strlen(word) - 1];
        word[strlen(word) - 1] = '\0';
    }
}

/* Build a tree starting at "row" in the wordle "w". 
 * Use the "parent" constraints to set up the constraints for this node
 * of the tree
 * For each word in "dict", 
 *    - if a word matches the constraints, then 
 *        - create a copy of the constraints for the child node and update
 *          the constraints with the new information.
 *        - add the word to the child_list of the current solver node
 *        - call solve_subtree on newly created subtree
 */

void solve_subtree(int row, struct wordle *w,  struct node *dict, 
                   struct solver_node *parent) {
    if(verbose) {
        printf("Running solve_subtree: %d, %s\n", row, parent->word);
    }

    // Set up parent constraints for root node
    if (row == 1){
        // Set cannot_be
        add_to_cannot_be(w->grid[row - 1], parent->con);
        // Update each row of must_be
        for (int i = 0; i < WORDLEN; i++){
            // Adding to must_be through set_green and set_yellow
            if (w->grid[row][i] == 'g'){
                set_green(parent->word[i], i, parent->con);
            }
            if (w->grid[row][i] == 'y') {
                set_yellow(i, w->grid[row], w->grid[row], parent->word, parent->con);
            }
        }
    }

    if(verbose) {
        print_constraints(parent->con);
    } 

    if (row < 0){
        return;
    }

    // If row == MAX_GUESSES - 1, then it is the first guess of the wordle, so there are no children
    if (row == MAX_GUESSES - 1){
        return;
    }

    struct node *curr = dict;
    //Iterate over the words in dict
    while (curr != NULL) {
        //If the current word matches the constraints
        if (match_constraints(curr->word, parent->con, w, row) == 1){
            //Create a new solver node with updated constraints
            struct solver_node *new_node = create_solver_node(parent->con, curr->word);
            //Update cannot_be
            add_to_cannot_be(curr->word, new_node->con);
            //Update each row of must_be
            for (int i = 0; i < WORDLEN; i++){
                //Reset must_be for the current row
                strncpy(new_node->con->must_be[i], "", SIZE);
                //Adding to must_be through set_green and set_yellow
                if (w->grid[row + 1][i] == 'g'){
                    set_green(curr->word[i], i, new_node->con);
                }
                if (w->grid[row + 1][i] == 'y') {
                    set_yellow(i, w->grid[row + 1], w->grid[row], curr->word, new_node->con);
                }
            }

            // Add this new word node to parent's child_list
            if (parent->child_list == NULL){
                parent->child_list = new_node;
            } else {
                // If this is not the first child, make parent's child_list point to the current child
                // and add have the current node point to the previous node as next_sibling
                new_node->next_sibling = parent->child_list;
                parent->child_list = new_node;
            }

            // Call solve_subtree on newly created subtree to create its children and siblings as well
            solve_subtree(row + 1, w, dict, new_node);
        }
        curr = curr->next;
    }
}

/* Print to standard output all paths that are num_rows in length.
 * - node is the current node for processing
 * - path is used to hold the words on the path while traversing the tree.
 * - level is the current length of the path so far.
 * - num_rows is the full length of the paths to print
 */

void print_paths(struct solver_node *node, char **path, 
                 int level, int num_rows) {

    // Add current word to path
    path[level - 1] = node->word;

    // Print once desired path length is reached
    if (level == num_rows){
        for (int i = 0; i < num_rows; i++){
            printf("%s ", path[i]);
        }
        printf("%c", '\n');
    }

    // Recurse into children on lower levels
    if (node->child_list != NULL){
        print_paths(node->child_list, path, level + 1, num_rows);
    }
    // Recurse over all nodes on the same level
    if (node->next_sibling != NULL){
        print_paths(node->next_sibling, path, level, num_rows);
    }
}

/* Free all dynamically allocated memory pointed to from w.
 */ 
void free_wordle(struct wordle *w){
    free(w);
}

/* Free all dynamically allocated pointed to from node
 */
void free_tree(struct solver_node *node){

    free(node->con);

    if (node->next_sibling != NULL) {
        free_tree(node->next_sibling);
    }
    if (node->child_list != NULL) {
        free_tree(node->child_list);
    }

    free(node);
}
