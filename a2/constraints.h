#include "common.h"
/* A data structure that holds a representation of the constraints
 * that will be used at a level of the grid.
 *   - must_be - a character array for each letter in the word that
 *           species which letters from the solution word must be in
 *           this slot.  It is used for yellow and green boxes
 *            - if must_be[i] is the empty string, then the letters in 
 *           this slot are limited by the letters that "cannot_be".  These
 *           are letters that apprear later in the solution path (earlier
 *           in the grid)  This is used for grey boxes.
 *   - cannot_be - a set of characters that cannot be in a grey box
 *     either because they are in the solution or they are in a word
 *     guessed in a row closer to the solution. 
 *     This set is represented as an array of 26 numbers.  If cannot_be[0] is 1
 *     then 'a' is in the set.  If cannot_be[1] is 0 then 'b' is not in the set.
 *     To add 'f' to the set, we can say "cannot_be['f'-'a'] = 1;"  We are  
 *     taking advantage of the fact that lower-case alphabet is stored as ascii 
 *     values in increasing order. ('f' - 'a' == 5)
 */
struct constraints {
    char must_be[WORDLEN][SIZE];  // if must_be[i] is '' then use cannot_be
    char cannot_be[ALPHABET_SIZE]; 
};

struct constraints *init_constraints();
void set_green(char letter, int index, struct constraints *con);
void set_yellow(int index, char *cur_tiles, char *next_tiles, 
                char *word, struct constraints *con);
void add_to_cannot_be(char *cur_word, struct constraints *con);
void print_constraints(struct constraints *c);

void free_constraints(struct constraints *c);
