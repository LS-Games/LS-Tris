#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/debug_log.h"

#include "round_controller.h"
#include "game_controller.h"
#include "play_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/round_dao_sqlite.h"

// Private functions
static char find_horizontal_winner(char board[BOARD_MAX]);
static char find_vertical_winner(char board[BOARD_MAX]);
static char find_diagonal_winner(char board[BOARD_MAX]);
static char find_winner(char board[BOARD_MAX]);
static bool is_draw(char board[BOARD_MAX]);
static bool is_valid_move(char board[BOARD_MAX], int row, int col);
static int get_current_turn(char* board);
static RoundControllerStatus round_end_helper(Round* roundToEnd, PlayResult result);

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

RoundControllerStatus round_get_public_info(int64_t id_round, RoundDTO **out_dto) {

    // Check if there's a roundwith this id_round
    Round retrievedRound;
    if (round_find_one(id_round, &retrievedRound) == ROUND_CONTROLLER_NOT_FOUND) {
        return ROUND_CONTROLLER_INVALID_INPUT;
    }

    RoundDTO *dynamicDTO = malloc(sizeof(RoundDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }

    map_round_to_dto(&retrievedRound, &(*dynamicDTO));
    
    *out_dto = dynamicDTO;
    
    return ROUND_CONTROLLER_OK;
}

RoundControllerStatus round_new(int64_t id_game, int64_t duration) {

    // Build empty board
    char emptyBoard[BOARD_MAX];
    fill_empty_board(emptyBoard);

    // Build round to start
    Round roundToStart = {
        .id_game = id_game,
        .duration = duration,
        .state = PENDING_ROUND,
    };
    strcpy(roundToStart.board, emptyBoard);
    
    LOG_STRUCT_DEBUG(print_round_inline, &roundToStart);
    
    // Create round
    return round_create(&roundToStart);
}

RoundControllerStatus round_start(int64_t id_round, int64_t id_playerOpponent) {

    // Retrieve round to start
    Round retrievedRound;
    RoundControllerStatus roundStatus = round_find_one(id_round, &retrievedRound);
    if (roundStatus != ROUND_CONTROLLER_OK)
        return roundStatus;

    // Retrieve game owner
    Game retrievedGame;
    GameControllerStatus gameStatus = game_find_one(retrievedRound.id_game, &retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    
    // Add plays to round
    PlayControllerStatus playStatus = play_add_round_plays(id_round, retrievedGame.id_owner, id_playerOpponent);
    if (playStatus != PLAY_CONTROLLER_OK)
        return ROUND_CONTROLLER_INTERNAL_ERROR;

    // Update round
    return round_update(&retrievedRound);
}

RoundControllerStatus round_make_move(int64_t id_round, int64_t id_playerMoving, int row, int col) {
    
    // Retrieve round
    Round retrievedRound;
    RoundControllerStatus roundStatus = round_find_one(id_round, &retrievedRound);
    if (roundStatus != ROUND_CONTROLLER_OK)
        return roundStatus;

    // Validate round
    if (retrievedRound.state != ACTIVE_ROUND)
        return ROUND_CONTROLLER_STATE_VIOLATION;

    // Retrieve current player_number
    int player_number;
    PlayControllerStatus playStatus = play_retrieve_current_player_number_of_round(id_round, id_playerMoving, &player_number);
    if (playStatus != PLAY_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_play_controller_status_to_string(playStatus));
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }
        
    // Check if it's the right turn
    if (player_number != get_current_turn(retrievedRound.board))
        return ROUND_CONTROLLER_FORBIDDEN;

    // Make move
    if (is_valid_move(retrievedRound.board, row, col)) {
        set_round_board_cell(retrievedRound.board, row, col, player_number_to_symbol(player_number));
    } else {
        return ROUND_CONTROLLER_INVALID_INPUT;
    }

    // Check win/draw conditions
    PlayResult result;
    char winner = find_winner(retrievedRound.board);
    if (winner == NO_SYMBOL) {
        if (is_draw(retrievedRound.board)) {
            result = DRAW;
        } else {
            result = PLAY_RESULT_INVALID;
        }
    } else {
        result = WIN;
    }

    // Update round
    RoundControllerStatus status = round_update(&retrievedRound);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    // If match is over
    if (result != PLAY_RESULT_INVALID) {
        return round_end_helper(&retrievedRound, result);
    }

    return ROUND_CONTROLLER_OK;
}

static RoundControllerStatus round_end_helper(Round* roundToEnd, PlayResult result) {
    
    // Set round status
    roundToEnd->state = FINISHED_ROUND;

    // If match is not a draw, check who won
    char winner = NO_SYMBOL;
    if (result != DRAW)
        winner = find_winner(roundToEnd->board);

    // Update play results
    PlayControllerStatus status = play_set_round_plays(roundToEnd->id_round, result, player_symbol_to_number(winner));
    if (status != PLAY_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_play_controller_status_to_string(status));
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }

    // Update round
    return round_update(roundToEnd);
}

RoundControllerStatus round_end(int64_t id_round) {
    
    // Retrieve round to end
    Round retrievedRound;
    RoundControllerStatus status = round_find_one(id_round, &retrievedRound);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    return round_end_helper(&retrievedRound, DRAW);
}

// ===================== CRUD Operations =====================

const char* return_round_controller_status_to_string(RoundControllerStatus status) {
    switch (status) {
        case ROUND_CONTROLLER_OK:               return "ROUND_CONTROLLER_OK";
        case ROUND_CONTROLLER_INVALID_INPUT:    return "ROUND_CONTROLLER_INVALID_INPUT";
        case ROUND_CONTROLLER_NOT_FOUND:        return "ROUND_CONTROLLER_NOT_FOUND";
        case ROUND_CONTROLLER_STATE_VIOLATION:  return "ROUND_CONTROLLER_STATE_VIOLATION";
        case ROUND_CONTROLLER_DATABASE_ERROR:   return "ROUND_CONTROLLER_DATABASE_ERROR";
        // case ROUND_CONTROLLER_CONFLICT:         return "ROUND_CONTROLLER_CONFLICT";
        case ROUND_CONTROLLER_FORBIDDEN:        return "ROUND_CONTROLLER_FORBIDDEN";
        case ROUND_CONTROLLER_INTERNAL_ERROR:   return "ROUND_CONTROLLER_INTERNAL_ERROR";
        default:                                return "ROUND_CONTROLLER_UNKNOWN";
    }
}

// Create
RoundControllerStatus round_create(Round* roundToCreate) {
    sqlite3* db = db_open();
    RoundDaoStatus status = insert_round(db, roundToCreate);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

// Read all
RoundControllerStatus round_find_all(Round** retrievedRoundArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    RoundDaoStatus status = get_all_rounds(db, retrievedRoundArray, retrievedObjectCount);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

// Read one
RoundControllerStatus round_find_one(int64_t id_round, Round* retrievedRound) {
    sqlite3* db = db_open();
    RoundDaoStatus status = get_round_by_id(db, id_round, retrievedRound);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return status == ROUND_DAO_NOT_FOUND ? ROUND_CONTROLLER_NOT_FOUND : ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

// Update
RoundControllerStatus round_update(Round* updatedRound) {
    sqlite3* db = db_open();
    RoundDaoStatus status = update_round_by_id(db, updatedRound);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

// Delete
RoundControllerStatus round_delete(int64_t id_round) {
    sqlite3* db = db_open();
    RoundDaoStatus status = delete_round_by_id(db, id_round);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return status == ROUND_DAO_NOT_FOUND ? ROUND_CONTROLLER_NOT_FOUND : ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}