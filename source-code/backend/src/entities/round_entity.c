#include <stdio.h>
#include <string.h>
#include <inttypes.h> // for PRId64
#include <time.h>

#include "../../include/debug_log.h"
#include "round_entity.h"

/*
 * Prints the round in a readable, multi-line format.
 * Time fields are printed as UNIX timestamps.
 */
void print_round(const Round *r) {
    if (!r) {
        printf("Round: (NULL)\n");
        return;
    }

    printf("Round {\n");
    printf("  id_round: %" PRId64 "\n", r->id_round);
    printf("  id_game: %" PRId64 "\n", r->id_game);
    printf("  state: \"%s\"\n", round_status_to_string(r->state));
    printf("  start_time: %" PRId64 "\n", r->start_time);
    printf("  end_time: %" PRId64 "\n", r->end_time);
    printf("  board: \"%s\"\n", r->board);

    // Print duration only if the round has ended
    if (r->end_time > 0 && r->start_time > 0) {
        printf("  duration_seconds: %" PRId64 "\n", r->end_time - r->start_time);
    }

    printf("}\n");
}

/*
 * Prints the round in a compact, single-line format.
 * Useful for logs and debugging.
 */
void print_round_inline(const Round *r) {
    if (!r) {
        printf("Round(NULL)\n");
        return;
    }

    int64_t duration = 0;
    if (r->end_time > 0 && r->start_time > 0) {
        duration = r->end_time - r->start_time;
    }

    printf(
        "Round[id=%" PRId64
        ", game=%" PRId64
        ", state=%s"
        ", start=%" PRId64
        ", end=%" PRId64
        ", dur=%" PRId64 "s"
        ", board=%s]\n",
        r->id_round,
        r->id_game,
        round_status_to_string(r->state),
        r->start_time,
        r->end_time,
        duration,
        r->board
    );
}

/*
 * Converts a RoundStatus enum to its string representation
 * used in the database.
 */
const char *round_status_to_string(RoundStatus state) {
    switch (state) {
        case ACTIVE_ROUND:    return "active";
        case PENDING_ROUND:   return "pending";
        case FINISHED_ROUND:  return "finished";
        default:              return "invalid";
    }
}

/*
 * Converts a string (from DB) to RoundStatus enum.
 */
RoundStatus string_to_round_status(const char *state_str) {
    if (!state_str) return ROUND_STATUS_INVALID;

    if (strcmp(state_str, "active") == 0)    return ACTIVE_ROUND;
    if (strcmp(state_str, "pending") == 0)   return PENDING_ROUND;
    if (strcmp(state_str, "finished") == 0)  return FINISHED_ROUND;

    return ROUND_STATUS_INVALID;
}

/*
 * Maps player number to board symbol.
 */
char player_number_to_symbol(int player_number) {
    switch (player_number) {
        case 1: return P1_SYMBOL;
        case 2: return P2_SYMBOL;
        default: return NO_SYMBOL;
    }
}

/*
 * Maps board symbol to player number.
 */
int player_symbol_to_number(const char player_symbol) {
    switch (player_symbol) {
        case P1_SYMBOL: return 1;
        case P2_SYMBOL: return 2;
        default: return -1;
    }
}

/*
 * Sets a symbol on the board at the given row and column.
 */
void set_round_board_cell(char board[BOARD_MAX], int row, int col, char symbol) {
    if (row < 0 || row >= BOARD_ROWS) {
        LOG_WARN("Provided row not valid!\n");
        return;
    }

    if (col < 0 || col >= BOARD_COLS) {
        LOG_WARN("Provided column not valid!\n");
        return;
    }

    board[BOARD_ROWS * row + col] = symbol;
}

/*
 * Returns the symbol at the given board position.
 */
char get_round_board_cell(char board[BOARD_MAX], int row, int col) {
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) {
        LOG_WARN("Invalid board coordinates!\n");
        return NO_SYMBOL;
    }

    return board[BOARD_ROWS * row + col];
}

/*
 * Initializes the board with empty symbols.
 */
void fill_empty_board(char board[BOARD_MAX]) {
    for (size_t i = 0; i < BOARD_MAX - 1; i++) {
        board[i] = EMPTY_SYMBOL;
    }
    board[BOARD_MAX - 1] = '\0';
}
