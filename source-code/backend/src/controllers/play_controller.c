#include <stdlib.h>

#include "../../include/debug_log.h"

#include "play_controller.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/play_dao_sqlite.h"

// This function provides a query by `id_player` and `id_round`. 
// @param id_player Possible values are all integer positive number and -1 (no filter)
// @param id_round Possible values are all integer positive number and -1 (no filter)
PlayControllerStatus plays_get_public_info(int64_t id_player, int64_t id_round, PlayDTO **out_dtos, int *out_count) {

    PlayWithPlayerNickname* retrievedPlays;
    int retrievedObjectCount;
    if (play_find_all_with_player_info(&retrievedPlays, &retrievedObjectCount) == PLAY_CONTROLLER_NOT_FOUND) {
        *out_dtos = NULL;
        *out_count = 0;
        return PLAY_CONTROLLER_NOT_FOUND;
    }

    PlayDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if ((id_player == -1 || retrievedPlays[i].id_player == id_player) &&
            (id_round == -1 || retrievedPlays[i].id_round == id_round)) {

            Play play = {
                .id_player = retrievedPlays[i].id_player,
                .id_round = retrievedPlays[i].id_round,
                .player_number = retrievedPlays[i].player_number,
                .result = retrievedPlays[i].result
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(PlayDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return PLAY_CONTROLLER_INTERNAL_ERROR;
            }

            map_play_to_dto(&play, retrievedPlays[i].player_nickname, &(dynamicDTOs[filteredObjectCount]));

            filteredObjectCount = filteredObjectCount + 1;
        }
    }

    *out_dtos = dynamicDTOs;
    *out_count = filteredObjectCount;
    
    return PLAY_CONTROLLER_OK;
}

// ===================== Controllers Helper Functions =====================

PlayControllerStatus play_add_round_plays(int64_t id_round, int64_t id_player_1, int64_t id_player_2) {
    
    // Build play of player 1
    Play playToBuild_1 = {
        .id_player = id_player_1,
        .id_round = id_round,
        .player_number = 1,
        .result = PLAY_RESULT_INVALID
    };

    // Build play of player 2
    Play playToBuild_2 = {
        .id_player = id_player_2,
        .id_round = id_round,
        .player_number = 2,
        .result = PLAY_RESULT_INVALID
    };
    
    // Create play
    PlayControllerStatus status = play_create(&playToBuild_1);
    if (status != PLAY_CONTROLLER_OK)
        return status;

    return play_create(&playToBuild_2);
}

// This function sets the plays of a round based on its result.
// @param result Possible values are `WIN` or `DRAW`
// @param winner If `result` is `WIN`, then winner represents the winner's player number. Otherwise it's ignored.
PlayControllerStatus play_set_round_plays_result(int64_t id_round, PlayResult result, int winner) {

    if (result != WIN && result != DRAW)
        return PLAY_CONTROLLER_INVALID_INPUT;
    
    // Retrieve plays of this round
    Play* retrievedPlayArray;
    int retrievedPlayCount;
    PlayControllerStatus playStatus = play_find_all_by_id_round(&retrievedPlayArray, id_round, &retrievedPlayCount);
    if (playStatus != PLAY_CONTROLLER_OK || retrievedPlayCount <= 0)
        return PLAY_CONTROLLER_INTERNAL_ERROR;

    // Set play status
    for (int i=0; i<retrievedPlayCount; i++) {
        if (result == DRAW) {
            retrievedPlayArray[i].result = DRAW;
        } else if (result == WIN) {
            if (retrievedPlayArray[i].player_number == winner) {
                retrievedPlayArray[i].result = WIN;
            } else {
                retrievedPlayArray[i].result = LOSE;
            }
        }

        // Update round
        playStatus = play_update(&retrievedPlayArray[i]);
        if (playStatus != PLAY_CONTROLLER_OK)
            return playStatus;
    }
    
    return PLAY_CONTROLLER_OK;
}

PlayControllerStatus play_retrieve_round_current_player_number(int64_t id_round, int64_t id_currentPlayer, int* out_player_number) {

    // Retrieve plays of this round
    Play* retrievedPlayArray;
    int retrievedPlayCount;
    PlayControllerStatus playStatus = play_find_all_by_id_round(&retrievedPlayArray, id_round, &retrievedPlayCount);
    if (playStatus != PLAY_CONTROLLER_OK)
        return playStatus;

    // Retrieve player_number
    int player_number = -1;
    for (int i=0; i < retrievedPlayCount; i++) {
        if (retrievedPlayArray[i].id_player == id_currentPlayer)
            player_number = retrievedPlayArray[i].player_number;
    }
    if (player_number == -1)
        return PLAY_CONTROLLER_INTERNAL_ERROR;

    *out_player_number = player_number;

    return PLAY_CONTROLLER_OK;
}

PlayControllerStatus play_find_round_winner(int64_t id_round, int64_t *out_id_playerWinner) {

    // Retrieve plays of this round
    Play* retrievedPlayArray;
    int retrievedPlayCount;
    PlayControllerStatus playStatus = play_find_all_by_id_round(&retrievedPlayArray, id_round, &retrievedPlayCount);
    if (playStatus != PLAY_CONTROLLER_OK || retrievedPlayCount <= 0)
        return PLAY_CONTROLLER_INTERNAL_ERROR;

    // Find winner
    int64_t id_winner = -1;
    for (int i=0; i < retrievedPlayCount; i++) {
        if (retrievedPlayArray[i].result == WIN) {
            id_winner = retrievedPlayArray[i].id_player;
        }
    }

    *out_id_playerWinner = id_winner;

    if (id_winner == -1)
        return PLAY_CONTROLLER_NOT_FOUND;
    
    return PLAY_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char *return_play_controller_status_to_string(PlayControllerStatus status) {
    switch (status) {
        case PLAY_CONTROLLER_OK:               return "PLAY_CONTROLLER_OK";
        case PLAY_CONTROLLER_INVALID_INPUT:    return "PLAY_CONTROLLER_INVALID_INPUT";
        case PLAY_CONTROLLER_NOT_FOUND:        return "PLAY_CONTROLLER_NOT_FOUND";
        // case PLAY_CONTROLLER_STATE_VIOLATION:  return "PLAY_CONTROLLER_STATE_VIOLATION";
        case PLAY_CONTROLLER_DATABASE_ERROR:   return "PLAY_CONTROLLER_DATABASE_ERROR";
        // case PLAY_CONTROLLER_CONFLICT:         return "PLAY_CONTROLLER_CONFLICT";
        // case PLAY_CONTROLLER_FORBIDDEN:        return "PLAY_CONTROLLER_FORBIDDEN";
        case PLAY_CONTROLLER_INTERNAL_ERROR:   return "PLAY_CONTROLLER_INTERNAL_ERROR";
        default:                                return "PLAY_CONTROLLER_UNKNOWN";
    }
}

// Create
PlayControllerStatus play_create(Play* playToCreate) {
    sqlite3* db = db_open();
    PlayDaoStatus status = insert_play(db, playToCreate);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read all
PlayControllerStatus play_find_all(Play **retrievedPlayArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayDaoStatus status = get_all_plays(db, retrievedPlayArray, retrievedObjectCount);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read one
PlayControllerStatus play_find_one(int64_t id_player, int64_t id_round, Play* retrievedPlay) {
    sqlite3* db = db_open();
    PlayDaoStatus status = get_play_by_pk(db, id_player, id_round, retrievedPlay);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return status == PLAY_DAO_NOT_FOUND ? PLAY_CONTROLLER_NOT_FOUND : PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Update
PlayControllerStatus play_update(Play* updatedPlay) {
    sqlite3* db = db_open();
    PlayDaoStatus status = update_play_by_pk(db, updatedPlay);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }
    
    return PLAY_CONTROLLER_OK;
}

// Delete
PlayControllerStatus play_delete(int64_t id_player, int64_t id_round) {
    sqlite3* db = db_open();
    PlayDaoStatus status = delete_play_by_pk(db, id_player, id_round);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return status == PLAY_DAO_NOT_FOUND ? PLAY_CONTROLLER_NOT_FOUND : PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read all (by id_round)
PlayControllerStatus play_find_all_by_id_round(Play **retrievedPlayArray, int64_t id_round, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayDaoStatus status = get_all_plays_by_round(db, retrievedPlayArray, id_round, retrievedObjectCount);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read all with player info
PlayControllerStatus play_find_all_with_player_info(PlayWithPlayerNickname **retrievedPlayArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayDaoStatus status = get_all_plays_with_player_info(db, retrievedPlayArray, retrievedObjectCount);
    db_close(db);
    if (status != PLAY_DAO_OK) {
        LOG_WARN("%s\n", return_play_dao_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}