#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/debug_log.h"

#include "round_controller.h"
#include "game_controller.h"
#include "play_controller.h"
#include "notification_controller.h"
#include "../json-parser/json-parser.h"
#include "../server/server.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/round_dao_sqlite.h"

// ==================== Private functions ====================

static char find_horizontal_winner(char board[BOARD_MAX]);
static char find_vertical_winner(char board[BOARD_MAX]);
static char find_diagonal_winner(char board[BOARD_MAX]);
static char find_winner(char board[BOARD_MAX]);
static bool is_draw(char board[BOARD_MAX]);
static bool is_valid_move(char board[BOARD_MAX], int row, int col);
static int get_current_turn(char *board);
static RoundControllerStatus round_start_helper(int64_t id_game, int64_t duration, Round* out_newRound);
static RoundControllerStatus round_end_helper(Round* roundToEnd, int64_t id_playerEndingRound, PlayResult result);

// ===========================================================

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

static int get_current_turn(char *board) {
    int p1 = 0, p2 = 0;

    for (int i = 0; i < BOARD_MAX; i++) {
        if (board[i] == P1_SYMBOL) p1++;
        else if (board[i] == P2_SYMBOL) p2++;
    }

    return p1 <= p2 ? 1 : 2;
}

RoundControllerStatus round_get_public_info(int64_t id_round, RoundDTO **out_dto, int *out_count) {

    // Check if there's a round with this id_round
    Round retrievedRound;
    if (round_find_one(id_round, &retrievedRound) == ROUND_CONTROLLER_NOT_FOUND) {
        *out_dto = NULL;
        *out_count = 0;
        return ROUND_CONTROLLER_NOT_FOUND;
    }

    RoundDTO *dynamicDTO = malloc(sizeof(RoundDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }

    map_round_to_dto(&retrievedRound, &(*dynamicDTO));
    
    *out_dto = dynamicDTO;
    *out_count = 1; 

    return ROUND_CONTROLLER_OK;
}

RoundControllerStatus round_make_move(int64_t id_round, int64_t id_playerMoving, int row, int col, int64_t* out_id_round) {

    // Retrieve round
    Round retrievedRound;
    LOG_INFO("ROUND ID IN ROUND MAKE MOVE: %d", id_round);
    RoundControllerStatus roundStatus = round_find_one(id_round, &retrievedRound);
    if (roundStatus != ROUND_CONTROLLER_OK)
        return roundStatus;

    // Validate round
    if (retrievedRound.state != ACTIVE_ROUND)
        return ROUND_CONTROLLER_STATE_VIOLATION;

    // Retrieve current player_number
    int player_number;
    PlayControllerStatus playStatus = play_retrieve_round_current_player_number(id_round, id_playerMoving, &player_number);
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

    // Send updated round move
    Play* retrievedPlayArray;
    int retrievedPlayCount;
    playStatus = play_find_all_by_id_round(&retrievedPlayArray, retrievedRound.id_round, &retrievedPlayCount);
    if (playStatus != PLAY_CONTROLLER_OK || retrievedPlayCount <= 0)
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    RoundDTO out_round_dto;
    map_round_to_dto(&retrievedRound, &out_round_dto);
    char *json_message = serialize_rounds_to_json("server_updated_round_move", &out_round_dto, 1);
    for (int i=0; i<retrievedPlayCount; i++) { // Send to all player except the player moving
        if (retrievedPlayArray[i].id_player != id_playerMoving)
            if (send_server_unicast_message(json_message, retrievedPlayArray[i].id_player) < 0 )
                return ROUND_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    // If match is over
    if (result != PLAY_RESULT_INVALID) {
        LOG_INFO("SONO ENTRATO IN MATCH OVER!");
        RoundStatus endStatus = round_end_helper(&retrievedRound, -1, result);
        *out_id_round = retrievedRound.id_round;

        return ROUND_CONTROLLER_OK;

    }

    *out_id_round = retrievedRound.id_round;

    return ROUND_CONTROLLER_OK;
}

RoundControllerStatus round_end(int64_t id_round, int64_t id_playerEndingRound, int64_t* out_id_round) {
    
    LOG_INFO("ID_ROUND PASSATO IN ROUND_END: %d", id_round);
    // Retrieve round to end
    Round retrievedRound;
    RoundControllerStatus status = round_find_one(id_round, &retrievedRound);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    status = round_end_helper(&retrievedRound, id_playerEndingRound, DRAW);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    *out_id_round = retrievedRound.id_round;

    return ROUND_CONTROLLER_OK;
}

static RoundControllerStatus round_end_helper(Round* roundToEnd, int64_t id_playerEndingRound, PlayResult result) {
    
    LOG_INFO("ROUND ID IN ROUND_END_HELPER: %d", roundToEnd->id_round);
    
    // Set round status
    roundToEnd->state = FINISHED_ROUND;

    // If match is not a draw, check who won
    char winner = NO_SYMBOL;
    if (result != DRAW)
        winner = find_winner(roundToEnd->board);

    // Update play results
    PlayControllerStatus playStatus = play_set_round_plays_result(roundToEnd->id_round, result, player_symbol_to_number(winner));
    if (playStatus != PLAY_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_play_controller_status_to_string(playStatus));
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }

    // Update game owner
    int64_t id_playerWinner = -1;
    playStatus = play_find_round_winner(roundToEnd->id_round, &id_playerWinner);
    if (playStatus != PLAY_CONTROLLER_NOT_FOUND) {
        GameControllerStatus gameStatus = game_change_owner(roundToEnd->id_game, id_playerWinner);
        if (gameStatus != GAME_CONTROLLER_OK) {
            LOG_WARN("%s\n", return_game_controller_status_to_string(gameStatus));
            return ROUND_CONTROLLER_INTERNAL_ERROR;
        }
    }

    // Update round
    RoundControllerStatus status = round_update(roundToEnd);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    // If there's a win, update the player causing the round end
    if (id_playerWinner != -1)
        id_playerEndingRound = id_playerWinner;

    LOG_INFO("PLAYER ENDING ROUND: %d", id_playerEndingRound);

    // Send notification
    NotificationDTO *out_notification_dto = NULL;
    if (notification_finished_round(roundToEnd->id_round, id_playerEndingRound, play_result_to_string(result), &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_round_end_notification", out_notification_dto);
    if (send_server_broadcast_message(json_message, id_playerEndingRound) < 0 ) {
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);
    free(out_notification_dto);

    // Send updated round end
    Play* retrievedPlayArray;
    int retrievedPlayCount;
    playStatus = play_find_all_by_id_round(&retrievedPlayArray, roundToEnd->id_round, &retrievedPlayCount);
    if (playStatus != PLAY_CONTROLLER_OK || retrievedPlayCount <= 0)
        return ROUND_CONTROLLER_INTERNAL_ERROR;
    RoundDTO out_round_dto;
    map_round_to_dto(roundToEnd, &out_round_dto);
    json_message = serialize_rounds_to_json("server_updated_round_end", &out_round_dto, 1);
    for (int i=0; i<retrievedPlayCount; i++) { // Send to all player except the player ending the round
        // if (retrievedPlayArray[i].id_player != id_playerEndingRound)
            if (send_server_unicast_message(json_message, retrievedPlayArray[i].id_player) < 0 )
                return ROUND_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    return ROUND_CONTROLLER_OK;
}

// ===================== Controllers Helper Functions =====================

RoundControllerStatus round_start(int64_t id_game, int64_t id_player1, int64_t id_player2, int64_t duration, int64_t *out_new_round) {

    // Build the new round
    Round out_newRound;
    RoundControllerStatus roundStatus = round_start_helper(id_game, duration, &out_newRound);
    if (roundStatus != ROUND_CONTROLLER_OK)
        return roundStatus;

    *out_new_round = out_newRound.id_round;

    // Add plays to round
    PlayControllerStatus playStatus = play_add_round_plays(out_newRound.id_round, id_player1, id_player2);
    if (playStatus != PLAY_CONTROLLER_OK)
        return ROUND_CONTROLLER_INTERNAL_ERROR;

    return ROUND_CONTROLLER_OK;
}

static RoundControllerStatus round_start_helper(int64_t id_game, int64_t duration, Round* out_newRound) {

    // Build empty board
    char emptyBoard[BOARD_MAX];
    fill_empty_board(emptyBoard);

    // Build new round
    Round roundToStart = {
        .id_game = id_game,
        .duration = duration,
        .state = ACTIVE_ROUND,
    };
    
    strcpy(roundToStart.board, emptyBoard);

    LOG_STRUCT_DEBUG(print_round_inline, &roundToStart);

    RoundControllerStatus status = round_create(&roundToStart);
    if (status != ROUND_CONTROLLER_OK)
        return status;

    LOG_DEBUG("AFTER INSERT: NEW ROUND ID = %" PRId64 "\n", roundToStart.id_round);

    *out_newRound = roundToStart;

    return ROUND_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char *return_round_controller_status_to_string(RoundControllerStatus status) {
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

    roundToCreate->id_round = sqlite3_last_insert_rowid(db);

    db_close(db);
    return ROUND_CONTROLLER_OK;
}

// Read all
RoundControllerStatus round_find_all(Round **retrievedRoundArray, int* retrievedObjectCount) {
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

    LOG_INFO("ROUND ID IN ROUND FIND ONE MOVE: %d", id_round);

    if (id_round <= 0) {
        LOG_WARN("round_make_move called with invalid id_round=%" PRId64, id_round);
        return ROUND_CONTROLLER_INVALID_INPUT;
    }

    sqlite3* db = db_open();
    RoundDaoStatus status = get_round_by_id(db, id_round, retrievedRound);
    db_close(db);

    LOG_INFO("STATUS: %s\n", return_round_dao_status_to_string(status));

    if (status == ROUND_DAO_NOT_MODIFIED) {
        LOG_INFO("No changes detected for round %d, skipping update.", retrievedRound->id_round);
        return GAME_CONTROLLER_OK;   
    }

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

RoundControllerStatus round_find_full_info_by_id_round(int64_t id_round, RoundFullDTO* retrievedFullRound) {
    sqlite3* db = db_open();
    RoundDaoStatus status = round_find_full_info(db, id_round, retrievedFullRound);
    db_close(db);
    if (status != ROUND_DAO_OK) {
        LOG_WARN("%s\n", return_round_dao_status_to_string(status));
        return status == ROUND_DAO_NOT_FOUND ? ROUND_CONTROLLER_NOT_FOUND : ROUND_CONTROLLER_DATABASE_ERROR;
    }

    return ROUND_CONTROLLER_OK;
}

