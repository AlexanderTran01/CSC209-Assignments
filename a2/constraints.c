#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "constraints.h"

/* Create and initialize a constraints struct. 
 * Sets the fields to 0 or empty string.
 * Return a pointer to the newly created constraints struct.
 */
struct constraints *init_constraints() {
    struct constraints *new_constraints = malloc(sizeof(struct constraints));

    //Setting fields to empty strings
    for (int i = 0; i < WORDLEN; i++){
        strncpy(new_constraints->must_be[i], "", SIZE);
    }
    //Setting fields to 0s
    for (int i = 0; i < ALPHABET_SIZE; i++){
        new_constraints->cannot_be[i] = 0;
    }

    return new_constraints;
}

/* Update the "must_be" field at "index" to be a string 
 * containing "letter"
 * The tile at this index is green, therefore the letter at "index"
 * must be "letter"
 */
void set_green(char letter, int index, struct constraints *con) {
    assert(islower(letter));
    assert(index >= 0 && index < WORDLEN);

    char l[2] = {letter, '\0'};
    strncpy(con->must_be[index], l, 1);
}

/* Update "con" by adding the possible letters to the string at the must_be 
 * field for "index".
 * - index is the index of the yellow tile in the current row to be updated
 * - cur_tiles is the tiles of this row
 * - next_tiles is the tiles of the row that is one closer to the solution row
 * - word is the word in the next row (assume word is all lower case letters)
 * Assume cur_tiles and next_tiles contain valid characters ('-', 'y', 'g')
 * 
 * Add to the must_be list for this index the letters that are green in the
 * next_tiles, but not green in the cur_tiles or green or yellow in the 
 * next_tiles at index.
 * Also add letters in yellow tiles in next_tiles.
 */
void set_yellow(int index, char *cur_tiles, char *next_tiles, 
                char *word, struct constraints *con) {

    assert(index >=0 && index < SIZE);
    assert(strlen(cur_tiles) == WORDLEN);
    assert(strlen(next_tiles) == WORDLEN);
    assert(strlen(word) == WORDLEN);

    //List of initially possible characters (before further constraints)
    char possible_chars[SIZE];
    possible_chars[0] = '\0';
    /*If a letter is either green or yellow in the next row then it is an
     *initial possibility for the letter at index i
     */
    for (int i = 0; i < WORDLEN; i++){
        if(next_tiles[i] == 'g' || next_tiles[i] == 'y'){
            char letter[2] = {word[i], '\0'};
            strncat(possible_chars, letter, 1);
        }
    }

    //List of impossible characters
    char impossible_chars[SIZE];
    impossible_chars[0] = '\0';
    //If a letter is green in the current row, it cannot be at index i
    for (int i = 0; i < WORDLEN; i++){
        if(cur_tiles[i] == 'g'){
            char letter[2] = {word[i], '\0'};
            strncat(impossible_chars, letter, 1);
        }
    }

    /*If a letter is green or yellow at index i in the next row, it cannot
     *be yellow at index i in the current row
     */
    if(next_tiles[index] == 'g' || next_tiles[index] == 'y'){
        char letter[2] = {word[index], '\0'};
        strncat(impossible_chars, letter, 1);
    }

    /*Filters out impossible characters from initially possible characters to
     *find list of possible characters after all constraints
     */
    for (int i = 0; i < strlen(possible_chars); i++){
        char *p = strchr(impossible_chars, possible_chars[i]);
        if(p == NULL){
            char letter[2] = {possible_chars[i], '\0'};
            strncat(con->must_be[index], letter, 1);
        }
    }
}

/* Add the letters from cur_word to the cannot_be field.
 * See the comments in constraints.h for how cannot_be is structured.
 */
void add_to_cannot_be(char *cur_word, struct constraints *con) {
    assert(strlen(cur_word) <= WORDLEN);

    for (int i = 0; i < strlen(cur_word); i++){
        char letter = cur_word[i];
        con->cannot_be[letter - 'a'] = 1;
    }
}

void print_constraints(struct constraints *c) {
    printf("cannot_be: ");

    for (int i = 0; i < ALPHABET_SIZE; i++){
        if (c->cannot_be[i] == 1){
            //represent lower case letters by adding 97 to i, as 97 == a
            printf("%c ", i + 97);
        }
    }
    
    printf("\nmust_be\n");

    for (int i = 0; i < WORDLEN; i++){
        printf("[%d] ", i);
        for (int j = 0; j < WORDLEN; j++){
            if (c->must_be[i][j] != '\0'){
                printf("%c ", c->must_be[i][j]);
            }
        }
        printf("\n");
    }
}