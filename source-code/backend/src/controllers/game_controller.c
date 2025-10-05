#include <stdlib.h>
#include <string.h>

#include "../../include/debug_log.h"

#include "game_controller.h"
#include "round_controller.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/game_dao_sqlite.h"

// This function provides a query by `status`. 
// @param status Possible values are `new`, `active`, `waiting`, `finished` and `all` (no filter)
GameControllerStatus games_get_public_info(GameDTO **out_dtos, char *status) {

    GameStatus queryStatus = GAME_STATUS_INVALID;
    if (strcmp(status, "all") != 0)
        queryStatus = string_to_game_status(status);
    if (queryStatus == GAME_STATUS_INVALID)
        return GAME_CONTROLLER_INVALID_INPUT;

    GameWithPlayerNickname* retrievedGames;
    int retrievedObjectCount;
    if (game_find_all_with_player_info(&retrievedGames, &retrievedObjectCount) == GAME_CONTROLLER_NOT_FOUND) {
        return GAME_CONTROLLER_INVALID_INPUT;
    }

    GameDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if (strcmp(status, "all") == 0 || retrievedGames[i].state == queryStatus) {

            Game game = {
                .id_game = retrievedGames[i].id_game,
                .state = retrievedGames[i].state,
                .created_at = retrievedGames[i].created_at
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(GameDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return GAME_CONTROLLER_INTERNAL_ERROR;
            }

            map_game_to_dto(&game, retrievedGames[i].creator, retrievedGames[i].owner, &(dynamicDTOs[filteredObjectCount]));

            filteredObjectCount = filteredObjectCount + 1;
        } 
    }

    *out_dtos = dynamicDTOs;
    
    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_start(int64_t id_creator) {

    // Build game to start
    Game gameToStart = {
        .id_creator = id_creator,
        .id_owner = id_creator,
        .created_at = time(NULL),
        .state = NEW_GAME
    };

    // Create game
    return game_create(&gameToStart);
}

GameControllerStatus game_end(int64_t id_game, int64_t id_owner) {

    // Retrieve game to end
    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    if (retrievedGame.id_owner != id_owner)
        return GAME_CONTROLLER_FORBIDDEN;

    retrievedGame.state = FINISHED_GAME;

    return game_update(&retrievedGame);
}

GameControllerStatus game_change_owner(int64_t id_game, int64_t id_newOwner) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    retrievedGame.id_owner = id_newOwner;

    return game_update(&retrievedGame);
}

// TODO: Scegliere se inserirlo nel controller delle notifications
GameControllerStatus game_send_rematch(int64_t id_game, int64_t id_playerSendingRematch, int64_t id_playerToRematch) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    if (id_playerSendingRematch == retrievedGame.id_owner) {
        // TODO: Invia notifica di rematch
    }

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_refuse_rematch(int64_t id_game) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Update game state in order to let anyone send participation requests
    retrievedGame.state = WAITING_GAME;

    return game_update(&retrievedGame);
}

GameControllerStatus game_accept_rematch(int64_t id_game, int64_t id_playerAcceptingRematch) {

    Game retrievedGame;
    GameControllerStatus gameStatus = game_find_one(id_game, &retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return gameStatus;
    
    // Start the round
    RoundControllerStatus roundStatus = round_start(retrievedGame.id_game, retrievedGame.id_owner, id_playerAcceptingRematch, 500);
    if (roundStatus != ROUND_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_round_controller_status_to_string(roundStatus));
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char* return_game_controller_status_to_string(GameControllerStatus status) {
    switch (status) {
        case GAME_CONTROLLER_OK:               return "GAME_CONTROLLER_OK";
        case GAME_CONTROLLER_INVALID_INPUT:    return "GAME_CONTROLLER_INVALID_INPUT";
        case GAME_CONTROLLER_NOT_FOUND:        return "GAME_CONTROLLER_NOT_FOUND";
        // case GAME_CONTROLLER_STATE_VIOLATION:  return "GAME_CONTROLLER_STATE_VIOLATION";
        case GAME_CONTROLLER_DATABASE_ERROR:   return "GAME_CONTROLLER_DATABASE_ERROR";
        // case GAME_CONTROLLER_CONFLICT:         return "GAME_CONTROLLER_CONFLICT";
        case GAME_CONTROLLER_FORBIDDEN:        return "GAME_CONTROLLER_FORBIDDEN";
        case GAME_CONTROLLER_INTERNAL_ERROR:   return "GAME_CONTROLLER_INTERNAL_ERROR";
        default:                                return "GAME_CONTROLLER_UNKNOWN";
    }
}

// Create
GameControllerStatus game_create(Game* gameToCreate) {
    sqlite3* db = db_open();
    GameDaoStatus status = insert_game(db, gameToCreate);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read all
GameControllerStatus game_find_all(Game** retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_all_games(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read one
GameControllerStatus game_find_one(int64_t id_game, Game* retrievedGame) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_game_by_id(db, id_game, retrievedGame);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Update
GameControllerStatus game_update(Game* updatedGame) {
    sqlite3* db = db_open();
    GameDaoStatus status = update_game_by_id(db, updatedGame);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Delete
GameControllerStatus game_delete(int64_t id_game) {
    sqlite3* db = db_open();
    GameDaoStatus status = delete_game_by_id(db, id_game);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read all with player info
GameControllerStatus game_find_all_with_player_info(GameWithPlayerNickname** retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_all_games_with_player_info(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}