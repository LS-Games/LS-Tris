#include <stdio.h>
#include <string.h>

#include "round_entity.h"

const char* round_status_to_string(RoundStatus state) {
    switch (state) {
        case ACTIVE_ROUND :               return "active";
        case PENDING_ROUND :              return "pending";
        case FINISHED_ROUND :             return "finished";
        default:                          return NULL;
    }
}

RoundStatus string_to_round_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "active") == 0)       return ACTIVE_ROUND;
        if (strcmp(state_str, "pending") == 0)      return PENDING_ROUND;
        if (strcmp(state_str, "finished") == 0)     return FINISHED_ROUND;
    }

    return ROUND_STATUS_INVALID;
}

void set_round_board_cell(char board[BOARD_MAX], int row, int col, char symbol) {
    if (0 <= row && row < BOARD_ROWS) {
        if (0 <= col && col < BOARD_COLS) {
            board[BOARD_ROWS*row + col] = symbol;
        } else {
            printf ("Provided column not valid!\n");
        }
        printf ("Provided row not valid!\n");
    }
}

char get_round_board_cell(char board[BOARD_MAX], int row, int col) {
    char cell = NO_SYMBOL;

    if (0 <= row && row < BOARD_ROWS) {
        if (0 <= col && col < BOARD_COLS) {
            cell = board[BOARD_ROWS*row + col];
        } else {
            printf ("Provided column not valid!\n");
        }
        printf ("Provided row not valid!\n");
    }

    return cell;
}