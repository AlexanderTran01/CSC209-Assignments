
#include "common.h"
#include "wordlist.h"
#define MAX_GUESSES 6

extern int verbose;

/* Contains the input information for this problem.
 * - num_rows is the number of rows in the solution grid
 * - grid[0] contains the solution string 
 * - grid contains the input grid of coloured boxes
 *       - "g" means the box is green
 *       - "y" means the box is yellow
 *       - "-" means the box is grey
 */
struct wordle {
    int num_rows;
    char grid[MAX_GUESSES][SIZE]; 
};


/* A node of the tree of word paths to a solution
 * - con is the constraints that will apply to build the list of 
 *   children to this node
 * - word is the word for this slot for the word path.
 * - next_sibling is a pointer to next potential word at this level.
 * - child_list is a pointer to the list of words that could precede
 *   word in the next row of the solution given the constraints for that row
 */
struct solver_node {
    struct constraints *con;
    char word[6];
    struct solver_node *next_sibling;
    struct solver_node *child_list;
};

struct wordle *create_wordle(FILE *fp);
struct solver_node *create_solver_node(struct constraints *c, char *word);
void solve_subtree(int row, struct wordle *w,  struct node *dict, struct solver_node *parent);
void print_paths(struct solver_node *node, char **path, int length, int num_rows);
struct solver_node *init_solution_node(char *word);

void free_wordle(struct wordle *w);
void free_tree(struct solver_node *tree);