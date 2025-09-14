#include <stdio.h>
#include <string.h>
#include <inttypes.h> // for PRId64

#include "../../include/debug_log.h"

#include "round_entity.h"

void print_round(const Round *r) {
    if (!r) {
        printf("Round: (NULL)\n");
        return;
    }

    printf("Round {\n");
    printf("  id_round: %" PRId64 "\n", r->id_round);
    printf("  id_game: %" PRId64 "\n", r->id_game);
    printf("  state: \"%s\"\n", round_status_to_string(r->state));
    printf("  duration: %" PRId64 "\n", r->duration);
    printf("  board: \"%s\"\n", r->board);
    printf("}\n");
}

void print_round_inline(const Round *r) {
    if (!r) {
        printf("Round(NULL)\n");
        return;
    }
    printf("Round[id=%" PRId64 ", game=%" PRId64 ", state=%s, dur=%" PRId64 ", board=%s]\n",
            r->id_round,
            r->id_game,
            round_status_to_string(r->state),
            r->duration,
            r->board);
}

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

char player_number_to_symbol(int player_number) {
    switch (player_number) {
        case 1:     return P1_SYMBOL;
        case 2:     return P2_SYMBOL;
        default:    return NO_SYMBOL;
    }
}

int player_symbol_to_number(const char player_symbol) {
    switch (player_symbol) {
        case P1_SYMBOL:     return 1;
        case P2_SYMBOL:     return 2;
        default:            return -1;
    }
}

void set_round_board_cell(char board[BOARD_MAX], int row, int col, char symbol) {
    if (0 <= row && row < BOARD_ROWS) {
        if (0 <= col && col < BOARD_COLS) {
            board[BOARD_ROWS*row + col] = symbol;
        } else {
            LOG_WARN("%s","Provided column not valid!\n");
        }
    } else {
        LOG_WARN("%s","Provided row not valid!\n");
    }
}

char get_round_board_cell(char board[BOARD_MAX], int row, int col) {
    char cell = NO_SYMBOL;

    if (0 <= row && row < BOARD_ROWS) {
        if (0 <= col && col < BOARD_COLS) {
            cell = board[BOARD_ROWS*row + col];
        } else {
            LOG_WARN("%s","Provided column not valid!\n");
        }
    } else {
        LOG_WARN("%s","Provided row not valid!\n");
    }

    return cell;
}