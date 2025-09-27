#include "../../include/debug_log.h"

#include "game_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/game_dao_sqlite.h"

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