#include "../../include/debug_log.h"

#include "game_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/game_dao_sqlite.h"

// ===================== CRUD Operations =====================

// Create
GameControllerStatus game_create(Game* gameToCreate) {
    sqlite3* db = db_open();
    GameReturnStatus status = insert_game(db, gameToCreate);
    db_close(db);
    if (status != GAME_OK) {
        LOG_WARN("%s\n", return_game_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }
    LOG_WARN("%p\n", (void *)gameToCreate);
    return GAME_CONTROLLER_OK;
}

// Read all
GameControllerStatus game_find_all(Game** retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameReturnStatus status = get_all_games(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_OK) {
        LOG_WARN("%s\n", return_game_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read one
GameControllerStatus game_find_one(int id_game, Game* retrievedGame) {
    sqlite3* db = db_open();
    GameReturnStatus status = get_game_by_id(db, id_game, retrievedGame);
    db_close(db);
    if (status != GAME_OK) {
        LOG_WARN("%s\n", return_game_status_to_string(status));
        return status == GAME_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Update
GameControllerStatus game_update(Game* updatedGame) {
    sqlite3* db = db_open();
    GameReturnStatus status = update_game_by_id(db, updatedGame);
    db_close(db);
    if (status != GAME_OK) {
        LOG_WARN("%s\n", return_game_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Delete
GameControllerStatus game_delete(int id_game) {
    sqlite3* db = db_open();
    GameReturnStatus status = delete_game_by_id(db, id_game);
    db_close(db);
    if (status != GAME_OK) {
        LOG_WARN("%s\n", return_game_status_to_string(status));
        return status == GAME_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}