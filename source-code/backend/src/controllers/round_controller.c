#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "round_controller.h"
#include "../db/sqlite/round_dao_sqlite.h"
#include "../db/sqlite/db_connection_sqlite.h"
//#include "../../include/debug_log.h" WIP

// Private functions
static char find_horizontal_winner(char board[BOARD_MAX]);
static char find_vertical_winner(char board[BOARD_MAX]);
static char find_diagonal_winner(char board[BOARD_MAX]);

static char find_horizontal_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // First row
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*0 + 1] && board[BOARD_ROWS*0 + 1] == board[BOARD_ROWS*0 + 2]
        && board[BOARD_ROWS*0 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*0 + 1];
    // Second row
    else if (board[BOARD_ROWS*1 + 0] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*1 + 2]
        && board[BOARD_ROWS*1 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 1];
    // Third row
    else if (board[BOARD_ROWS*2 + 0] == board[BOARD_ROWS*2 + 1] && board[BOARD_ROWS*2 + 1] == board[BOARD_ROWS*2 + 2]
        && board[BOARD_ROWS*2 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*2 + 1];

    return winner;        
}

static char find_vertical_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // First column
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*1 + 0] && board[BOARD_ROWS*1 + 0] == board[BOARD_ROWS*2 + 0]
        && board[BOARD_ROWS*1 + 0] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 0];
    // Second column
    else if (board[BOARD_ROWS*0 + 1] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 1]
        && board[BOARD_ROWS*1 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 1];
    // Third column
    else if (board[BOARD_ROWS*0 + 2] == board[BOARD_ROWS*1 + 2] && board[BOARD_ROWS*1 + 2] == board[BOARD_ROWS*2 + 2]
        && board[BOARD_ROWS*1 + 2] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 2];

    return winner;        
}

static char find_diagonal_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // Principal diagonal
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 2]
        && board[BOARD_ROWS*1 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 1];
    // Secondary diagonal
    else if (board[BOARD_ROWS*0 + 2] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 0]
        && board[BOARD_ROWS*1 + 1] != EMPTY_SYMBOL)
        winner = board[BOARD_ROWS*1 + 1];

    return winner;        
}

char find_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    if (strlen(board)+1 == BOARD_MAX) { // strlen(board) will return the number of char (without trailing '\0')
        winner = find_horizontal_winner(board);
    
        if (winner == NO_SYMBOL) {
            winner = find_vertical_winner(board);
    
            if (winner == NO_SYMBOL ) {
                winner = find_diagonal_winner(board);
            }
        }
    } else {
        printf("This board dimension is not a valid! It should be a %dx%d board.\n", BOARD_ROWS, BOARD_COLS);
    }

    return winner;
}

bool is_valid_move(char board[BOARD_MAX], int row, int col) {
    bool valid = false;

    char cell = get_round_board_cell(board, row, col);

    if (cell == EMPTY_SYMBOL)
        valid = true;

    return valid;
}

bool make_move(char board[BOARD_MAX], int row, int col, char symbol) {
    bool success = false;

    if (is_valid_move(board, row, col)) {
        set_round_board_cell(board, row, col, symbol);
        success = true;
    }

    return success;
}

bool start_round(int id_game, int64_t duration) {
    sqlite3* db = db_open();

    Round round = {
        .id_game = id_game,
        .duration = duration,
        .state = PENDING_ROUND,
        .board = "@@@@@@@@@"
    };

    RoundReturnStatus status = insert_round(db, &round, NULL);

    db_close(db);

    // Something went wrong
    if (status != ROUND_OK)
        printf("%s\n", return_round_status_to_string(status));

    //LOG_STRUCT_DEBUG(print_round, &round); WIP

    return status==ROUND_OK;
}

bool end_round(int id_round) {
    sqlite3* db = db_open();

    Round round;
    RoundReturnStatus status = get_round_by_id(db, id_round, &round);

    if (status == ROUND_OK) {
        round.state = FINISHED_ROUND;

        update_round_by_id(db, &round);
    }

    db_close(db);

    // Something went wrong
    if (status != ROUND_OK)
        printf("%s\n", return_round_status_to_string(status));

    return status==ROUND_OK;
}