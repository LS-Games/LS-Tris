#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../../include/debug_log.h"

#include "round_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/round_dao_sqlite.h"
#include "../db/sqlite/play_dao_sqlite.h"

// Private functions
static char find_horizontal_winner(char board[BOARD_MAX]);
static char find_vertical_winner(char board[BOARD_MAX]);
static char find_diagonal_winner(char board[BOARD_MAX]);
static char find_winner(char board[BOARD_MAX]);
static bool is_draw(char board[BOARD_MAX]);
static bool is_valid_move(char board[BOARD_MAX], int row, int col);
static int get_current_turn(char* board);
static RoundControllerStatus round_end_helper(Round round);

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

static char find_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    if (strlen(board)+1 == BOARD_MAX) { // strlen(board) will return the number of char (without trailing '\0')
        winner = find_horizontal_winner(board);
    
        if (winner == NO_SYMBOL) {
            winner = find_vertical_winner(board);
    
            if (winner == NO_SYMBOL) {
                winner = find_diagonal_winner(board);
            }
        }
    } else {
        LOG_WARN("This board has not a valid dimension! It should be a %dx%d board.\n", BOARD_ROWS, BOARD_COLS);
    }

    return winner;
}

static bool is_draw(char board[BOARD_MAX]) {
    return strchr(board, EMPTY_SYMBOL) == NULL;
}

static bool is_valid_move(char board[BOARD_MAX], int row, int col) {
    bool valid = false;

    char cell = get_round_board_cell(board, row, col);

    if (cell == EMPTY_SYMBOL)
        valid = true;

    return valid;
}

static int get_current_turn(char* board) {
    int p1SymbolCounter = 0;
    int p2SymbolCounter = 0;
    
    while (board) {
        if (*board == P1_SYMBOL) {
            p1SymbolCounter++;
        } else if (*board == P2_SYMBOL) {
            p2SymbolCounter++;
        }

        board++;
    }

    return p1SymbolCounter <= p2SymbolCounter ? 1 : 2;
}

RoundControllerStatus round_start(int id_game, int64_t duration) {

    Round round = {
        .id_game = id_game,
        .duration = duration,
        .state = PENDING_ROUND,
        .board = EMPTY_BOARD
    };
    
    LOG_STRUCT_DEBUG(print_round_inline, &round);
    
    // Insert round
    sqlite3* db = db_open();
    RoundReturnStatus status = insert_round(db, &round, NULL);
    db_close(db);
    if (status != ROUND_OK) {
        LOG_WARN("%s\n", return_round_status_to_string(status));
        return ROUND_CONTROLLER_PERSISTENCE_ERROR;
    }

    LOG_STRUCT_DEBUG(print_round_inline, &round);

    return ROUND_CONTROLLER_OK;
}

RoundControllerStatus round_make_move(int id_round, int id_player, int row, int col) {
    
    // Retrieve round
    Round round;
    sqlite3* db = db_open();
    RoundReturnStatus roundStatus = get_round_by_id(db, id_round, &round);
    db_close(db);
    if (roundStatus != ROUND_OK) {
        LOG_WARN("%s\n", return_round_status_to_string(roundStatus));
        return ROUND_CONTROLLER_NOT_FOUND;
    }

    // Validate round
    if (round.state != ACTIVE_ROUND) {
        return ROUND_CONTROLLER_STATE_VIOLATION;
    }

    // Retrieve play
    Play play;
    db = db_open();
    PlayReturnStatus playStatus = get_play_by_pk(db, id_player, id_round, &play);
    db_close(db);
    if (playStatus != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(playStatus));
        return ROUND_CONTROLLER_NOT_FOUND;
    }

    /* Retrieve player_number
    //  Dobbiamo scegliere se aggiungere a Round:
            - player_1: player_id
            - player_2: player_id
        Oppure a Play:
            - player_number: int
    player_id_t current_turn_player;
    if (ctrl->round_repo.get_current_turn(round_id, &current_turn_player) != ROUND_OK) {
        return UC_INTERNAL_ERROR;
    }

    // 4. Validazione: è il turno giusto?
    if (player_id != current_turn_player) {
        return UC_FORBIDDEN;
    }
    */

    // Make move
    if (is_valid_move(round.board, row, col)) {
        /*
        // 6. Aggiorna la board con il simbolo del giocatore
        round.board[cell_index] = (player_id == ctrl->player1_id) ? 'X' : 'O';
        */
        //set_round_board_cell(round.board, row, col, symbol);
    } else {
        return ROUND_CONTROLLER_INVALID_INPUT;
    }

    // Check win/draw conditions
    PlayResult result;
    char winner = find_winner(round.board);
    if (winner == NO_SYMBOL) {
        if (is_draw(round.board)) {
            result = DRAW;
        } else {
            result = PLAY_RESULT_INVALID;
        }
    } else {
        result = WIN;
    }

    // If match is over
    if (result != PLAY_RESULT_INVALID) {
        return round_end_helper(round);
    }

    return ROUND_CONTROLLER_OK;
}

static RoundControllerStatus round_end_helper(Round round) {
    round.state = FINISHED_ROUND;
    /*
    ctrl->play_repo.record_result(round_id, player_id, result);
    // Registra anche il risultato per l’altro giocatore
    player_id_t other = (player_id == ctrl->player1_id) ? ctrl->player2_id : ctrl->player1_id;
    PlayResult other_result = (result == WIN) ? LOSE : (result == LOSE) ? WIN : DRAW;
    ctrl->play_repo.record_result(round_id, other, other_result);
    */

    // Update round
    sqlite3* db = db_open();
    RoundReturnStatus status = update_round_by_id(db, &round);
    db_close(db);
    if (status != ROUND_OK) {
        LOG_WARN("%s\n", return_round_status_to_string(status));   
        return ROUND_CONTROLLER_PERSISTENCE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

RoundControllerStatus round_end(int id_round) {
    Round round;
    sqlite3* db = db_open();
    RoundReturnStatus status = get_round_by_id(db, id_round, &round);
    db_close(db);
    if (status != ROUND_OK) {
        LOG_WARN("%s\n", return_round_status_to_string(status));
        return ROUND_CONTROLLER_NOT_FOUND;
    }

    return round_end_helper(round);
}