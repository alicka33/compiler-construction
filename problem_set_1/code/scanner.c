#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// TODO: Update to your state values
#define N_STATES 12
#define START_STATE 0
#define ACCEPT 10
#define ERROR 11

#define DASH 45  // '-'
#define DOT 46   // '.'
#define DIGIT_BEG 48  // '0'
#define DIGIT_END 57  // '9'
#define NL 10  // /n 

#define D 'd'
#define X 'x'
#define Y 'y'
#define G 'g'
#define O 'o'
#define EQUAL '='

int transition_table[N_STATES][256]; // Table form of the automaton

void initialize_transition_table() {
    // TODO: Fill the transition table with values

    // all values set to error at the beginning 
    for (int i = 0; i < N_STATES; i++) {
        for (int j = 0; j<256; j++){
            transition_table[i][j] = ERROR;
        }
    }

    // assigning what symbol will cause a given state to change into another state
    //0
    transition_table[0][D] = 1;
    transition_table[0][G] = 8;
    //1
    transition_table[1][X] = 2;
    transition_table[1][Y] = 3;
    //2
    transition_table[2][EQUAL] = 4;
    //3
    transition_table[3][EQUAL] = 4;
    //4
    transition_table[4][DASH] = 5;
    transition_table[4][DOT] = 6;
    for (int i = DIGIT_BEG; i < DIGIT_END+1; i++) 
        transition_table[4][i] = 5;
    //5
    transition_table[5][DOT] = 6;
    for (int i = DIGIT_BEG; i< DIGIT_END+1; i++) 
        transition_table[5][i] = 5;
    //6
    for (int i = DIGIT_BEG; i< DIGIT_END+1; i++) 
        transition_table[6][i] = 7;
    //7
    transition_table[7][NL] = ACCEPT;
    for (int i = DIGIT_BEG; i< DIGIT_END+1;i++) 
        transition_table[7][i] = 7;
    //8
    transition_table[8][O] = 9;
    //9
    transition_table[9][NL] = ACCEPT;
    //10
    // 10 is the ACCEPTING state
    //11
    // 11 is the error state in my algorithm
}

// Driver program's internal state
int state = START_STATE;
float x = 421, y = 298,    // We start at the middle of the page,
      dx = 0, dy = 0;      // and with dx=dy=0

// Used to store the chars of statement we are currently reading
char lexeme_buffer[1024];
int lexeme_length = 0;

// In here we can assume that lexeme_buffer contains a valid statement, since the DFA reached ACCEPT
void handle_statement() {
    if (strncmp(lexeme_buffer, "go", 2) == 0) {
        x = x + dx;
        y = y + dy;
        printf( "%f %f lineto\n", x, y );
        printf( "%f %f moveto\n", x, y );
    } else if (strncmp(lexeme_buffer, "dx=", 3) == 0) {
        sscanf( lexeme_buffer+3, "%f", &dx );
    } else if (strncmp(lexeme_buffer, "dy=", 3) == 0) {
        sscanf( lexeme_buffer+3, "%f", &dy );
    } else {
        assert(0 && "Reached an unreachable branch!");
    }
}

int main() {
    // Setup the DFA transitions as a table
    initialize_transition_table();

    // PostScript preable to create a valid ps-file
    printf ( "<< /PageSize [842 595] >> setpagedevice\n" );
    printf ( "%f %f moveto\n", x, y );

    // Main loop
    int line_num = 1; // Used to report which line an error occured on
    int read;
    while( (read = getchar()) != EOF) {
        // Store the read char in the buffer
        lexeme_buffer[lexeme_length++] = read;
        lexeme_buffer[lexeme_length] = 0; // Add NULL terminator

        // Use the current state and the read char to find the next state
        state = transition_table[state][read];

        // Check if we reached the ACCEPT or ERROR states
        switch (state) {
            case ACCEPT:
                handle_statement();
                state = START_STATE;
                lexeme_length = 0;
                break;
            case ERROR:
                fprintf(stderr, "error: %d: unrecognized statement: %s\n", line_num, lexeme_buffer);
                exit( EXIT_FAILURE );
            default: break;
        }

        // If the char was a newline, the next char will be on a new line!
        if (read == '\n')
            line_num++;
    }

    if (state != START_STATE) {
        fprintf(stderr, "error: %d: input ended in the middle of a statement: %s\n", line_num, lexeme_buffer);
        exit( EXIT_FAILURE );
    }

    printf ( "stroke\n" );
    printf ( "showpage\n" );
}
