#ifndef SEQBOT_HELPERS
#define SEQBOT_HELPERS

// calculate the melting tempearture for the dna sequence
int calculate_melting_temperature(char *sequence, int sequence_length);

// print the instructions to synthesize the given DNA sequence to stdout
void print_instructions(char *sequence, int sequence_length);

// print the sequences for all possible molecules of length k
void generate_all_molecules(int k);

// generate the instructions for the molecules described in filename
void generate_molecules_from_file(char* filename);

#endif
