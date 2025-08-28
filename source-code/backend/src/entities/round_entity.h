#ifndef ROUND_ENTITY_H
#define ROUND_ENTITY_H

#include <stdint.h>

#define BOARD_ROWS 3
#define BOARD_COLS 3 
#define BOARD_MAX BOARD_ROWS*BOARD_COLS

#define NO_SYMBOL '/'

typedef enum {
    ACTIVE_ROUND,
    PENDING_ROUND,
    FINISHED_ROUND,
    ROUND_STATUS_INVALID
} RoundStatus;

typedef struct {
    int id_round;
    int id_game;
    RoundStatus state;
    int64_t duration;
    char board[BOARD_MAX];
} Round;

const char* round_status_to_string(RoundStatus state);
RoundStatus string_to_round_status(const char *state_str);

void set_round_board_cell(char board[BOARD_MAX], int row, int col, char symbol);
char get_round_board_cell(char board[BOARD_MAX], int row, int col);

#endif