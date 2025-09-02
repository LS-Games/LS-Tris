#ifndef ROUND_ENTITY_H
#define ROUND_ENTITY_H

#include <stdint.h>

#define BOARD_ROWS 3
#define BOARD_COLS 3 
#define BOARD_MAX (BOARD_ROWS*BOARD_COLS)+1 // One more character for trailing \0 char

#define NO_SYMBOL '/'
#define P1_SYMBOL 'X'
#define P2_SYMBOL 'O'
#define EMPTY_SYMBOL '@'

#define EMPTY_BOARD "@@@@@@@@@"

typedef enum {
    ACTIVE_ROUND,
    PENDING_ROUND,
    FINISHED_ROUND,
    ROUND_STATUS_INVALID
} RoundStatus;

typedef struct {
    int64_t id_round;
    int64_t id_game;
    RoundStatus state;
    int64_t duration;
    char board[BOARD_MAX];
} Round;

void print_round(const Round *r);
void print_round_inline(const Round *r);

const char* round_status_to_string(RoundStatus state);
RoundStatus string_to_round_status(const char *state_str);

void set_round_board_cell(char board[BOARD_MAX], int row, int col, char symbol);
char get_round_board_cell(char board[BOARD_MAX], int row, int col);

#endif