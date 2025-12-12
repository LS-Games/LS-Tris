#include <stdlib.h>
#include <string.h>

#include "../../include/debug_log.h"

#include "game_controller.h"
#include "round_controller.h"
#include "player_controller.h"
#include "notification_controller.h"
#include "../json-parser/json-parser.h"
#include "../server/server.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/game_dao_sqlite.h"

// This function provides a query by `status`. 
// @param status Possible values are `new`, `active`, `waiting`, `finished` and `all` (no filter)
GameControllerStatus games_get_public_info(char *status, GameDTO **out_dtos, int *out_count) {

    LOG_DEBUG("Status: %s\n", status);

    GameStatus queryStatus = GAME_STATUS_INVALID;
    if (strcmp(status, "all") != 0) {
        queryStatus = string_to_game_status(status);
        if (queryStatus == GAME_STATUS_INVALID)
            return GAME_CONTROLLER_INVALID_INPUT;
    }

    GameWithPlayerNickname* retrievedGamesWithPlayerInfo;
    int retrievedObjectCount;
    if (game_find_all_with_player_info(&retrievedGamesWithPlayerInfo, &retrievedObjectCount) == GAME_CONTROLLER_NOT_FOUND) {
        *out_dtos = NULL;
        *out_count = 0;
        return GAME_CONTROLLER_NOT_FOUND;
    }

    GameDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if (strcmp(status, "all") == 0 || retrievedGamesWithPlayerInfo[i].state == queryStatus) {

            Game game = {
                .id_game = retrievedGamesWithPlayerInfo[i].id_game,
                .state = retrievedGamesWithPlayerInfo[i].state,
                .created_at = retrievedGamesWithPlayerInfo[i].created_at
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(GameDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return GAME_CONTROLLER_INTERNAL_ERROR;
            }

            map_game_to_dto(&game, retrievedGamesWithPlayerInfo[i].creator, retrievedGamesWithPlayerInfo[i].owner, &(dynamicDTOs[filteredObjectCount]));

            filteredObjectCount = filteredObjectCount + 1;
        } 
    }

    *out_dtos = dynamicDTOs;
    *out_count = filteredObjectCount;
    
    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_start(int64_t id_creator, int64_t* out_id_game) {

    // Build game to start
    Game gameToStart = {
        .id_creator = id_creator,
        .id_owner = id_creator,
        .created_at = time(NULL),
        .state = NEW_GAME
    };

    // Create game
    GameControllerStatus status = game_create(&gameToStart);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send notification
    NotificationDTO *out_notification_dto = NULL;
    if (notification_new_game(gameToStart.id_game, id_creator, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_start_notification", out_notification_dto);
    if (send_server_broadcast_message(json_message, id_creator) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);
    free(out_notification_dto);

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(gameToStart.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&gameToStart, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    json_message = serialize_games_to_json("server_new_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, gameToStart.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = gameToStart.id_game;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_end(int64_t id_game, int64_t id_owner, int64_t* out_id_game) {

    // Retrieve game to end
    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    if (retrievedGame.id_owner != id_owner)
        return GAME_CONTROLLER_FORBIDDEN;

    retrievedGame.state = FINISHED_GAME;

    status = game_update(&retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(retrievedGame.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&retrievedGame, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    char *json_message = serialize_games_to_json("server_end_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = retrievedGame.id_game;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_refuse_rematch(int64_t id_game, int64_t* out_id_game) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Update game state in order to let anyone send participation requests
    retrievedGame.state = WAITING_GAME;

    status = game_update(&retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send notification
    NotificationDTO *out_notification_dto = NULL;
    if (notification_waiting_game(retrievedGame.id_game, retrievedGame.id_owner, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_waiting_notification", out_notification_dto);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);
    free(out_notification_dto);

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(retrievedGame.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&retrievedGame, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    json_message = serialize_games_to_json("server_waiting_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = retrievedGame.id_game;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_accept_rematch(int64_t id_game, int64_t id_playerAcceptingRematch, int64_t* out_id_game) {

    Game retrievedGame;
    GameControllerStatus gameStatus = game_find_one(id_game, &retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return gameStatus;

    int64_t new_round_id;
    
    // Start the round
    RoundControllerStatus roundStatus = round_start(retrievedGame.id_game, retrievedGame.id_owner, id_playerAcceptingRematch, 500, &new_round_id);
    if (roundStatus != ROUND_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_round_controller_status_to_string(roundStatus));
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    *out_id_game = retrievedGame.id_game;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_cancel(int64_t id_game, int64_t id_owner, int64_t* out_id_game) {

    NotificationDTO *out_notification_dto = NULL;
    if(notification_game_cancel(id_game, id_owner, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_cancel", out_notification_dto);
    if (send_server_broadcast_message(json_message, id_owner) < 0 ) {
        free(json_message);
        free(out_notification_dto);
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    free(json_message);
    free(out_notification_dto);

    GameControllerStatus status = game_delete(id_game);

    if (status != GAME_CONTROLLER_OK)
        return status;

    *out_id_game = id_game;

    return GAME_CONTROLLER_OK;
}

// ===================== Controllers Helper Functions =====================

GameControllerStatus game_change_owner(int64_t id_game, int64_t id_newOwner) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    retrievedGame.id_owner = id_newOwner;

    return game_update(&retrievedGame);
}

// ===================== CRUD Operations =====================

const char *return_game_controller_status_to_string(GameControllerStatus status) {
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
GameControllerStatus game_find_all(Game **retrievedGameArray, int* retrievedObjectCount) {
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
    LOG_INFO("UPDATE GAME_ID: %d", updatedGame->id_game);
    GameDaoStatus status = update_game_by_id(db, updatedGame);
    db_close(db);

    if (status == GAME_DAO_NOT_MODIFIED) {
        LOG_INFO("No changes detected for game %d, skipping update.", updatedGame->id_game);
        return GAME_CONTROLLER_OK;     // <-- NON è un errore
    }

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

// Read one with player info
GameControllerStatus game_find_one_with_player_info(int64_t id_game, GameWithPlayerNickname* retrievedGame) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_game_by_id_with_player_info(db, id_game, retrievedGame);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read all with player info
GameControllerStatus game_find_all_with_player_info(GameWithPlayerNickname **retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_all_games_with_player_info(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}