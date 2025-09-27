#include <stdlib.h>

#include "../../include/debug_log.h"

#include "game_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/game_dao_sqlite.h"

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

GameControllerStatus games_get_public_info(GameDTO **out_dtos) {

    GameWithPlayerNickname* retrievedGames;
    int retrievedObjectCount;
    if (game_find_all_with_player_info(&retrievedGames, &retrievedObjectCount) == GAME_CONTROLLER_NOT_FOUND) {
        return GAME_CONTROLLER_INVALID_INPUT;
    }

    GameDTO *dynamicDtos = malloc(sizeof(GameDTO) * retrievedObjectCount);

    if (dynamicDtos == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    for (int i = 0; i < retrievedObjectCount; i++) {
        Game game = {
            .id_game = retrievedGames[i].id_game,
            .state = retrievedGames[i].state,
            .created_at = retrievedGames[i].created_at
        };

        map_game_to_dto(&game, retrievedGames[i].creator, retrievedGames[i].owner, &(dynamicDtos[i]));
    }

    *out_dtos = dynamicDtos;
    
    return GAME_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char* return_game_controller_status_to_string(GameControllerStatus status) {
    switch (status) {
        case GAME_CONTROLLER_OK:               return "GAME_CONTROLLER_OK";
        // case GAME_CONTROLLER_INVALID_INPUT:    return "GAME_CONTROLLER_INVALID_INPUT";
        case GAME_CONTROLLER_NOT_FOUND:        return "GAME_CONTROLLER_NOT_FOUND";
        // case GAME_CONTROLLER_STATE_VIOLATION:  return "GAME_CONTROLLER_STATE_VIOLATION";
        case GAME_CONTROLLER_DATABASE_ERROR:   return "GAME_CONTROLLER_DATABASE_ERROR";
        // case GAME_CONTROLLER_CONFLICT:         return "GAME_CONTROLLER_CONFLICT";
        // case GAME_CONTROLLER_FORBIDDEN:        return "GAME_CONTROLLER_FORBIDDEN";
        // case GAME_CONTROLLER_INTERNAL_ERROR:   return "GAME_CONTROLLER_INTERNAL_ERROR";
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
GameControllerStatus game_find_one(int id_game, Game* retrievedGame) {
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
GameControllerStatus game_delete(int id_game) {
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