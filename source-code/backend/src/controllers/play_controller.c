#include "../../include/debug_log.h"

#include "play_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/play_dao_sqlite.h"

// ===================== CRUD Operations =====================

// Create
PlayControllerStatus play_create(Play* playToCreate) {
    sqlite3* db = db_open();
    PlayReturnStatus status = insert_play(db, playToCreate);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }
    LOG_WARN("%p\n", (void *)playToCreate);
    return PLAY_CONTROLLER_OK;
}

// Read all
PlayControllerStatus play_find_all(Play** retrievedPlayArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayReturnStatus status = get_all_plays(db, retrievedPlayArray, retrievedObjectCount);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read one
PlayControllerStatus play_find_one(int id_play, int id_round, Play* retrievedPlay) {
    sqlite3* db = db_open();
    PlayReturnStatus status = get_play_by_pk(db, id_play, id_round, retrievedPlay);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return status == PLAY_NOT_FOUND ? PLAY_CONTROLLER_NOT_FOUND : PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Update
PlayControllerStatus play_update(Play* updatedPlay) {
    sqlite3* db = db_open();
    PlayReturnStatus status = update_play_by_pk(db, updatedPlay);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }
    
    return PLAY_CONTROLLER_OK;
}

// Delete
PlayControllerStatus play_delete(int id_play, int id_round) {
    sqlite3* db = db_open();
    PlayReturnStatus status = delete_play_by_pk(db, id_play, id_round);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return status == PLAY_NOT_FOUND ? PLAY_CONTROLLER_NOT_FOUND : PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}

// Read all (by_round)
PlayControllerStatus play_find_all_by_round(Play** retrievedPlayArray, int64_t id_round, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayReturnStatus status = get_all_plays_by_round(db, retrievedPlayArray, id_round, retrievedObjectCount);
    db_close(db);
    if (status != PLAY_OK) {
        LOG_WARN("%s\n", return_play_status_to_string(status));
        return PLAY_CONTROLLER_DATABASE_ERROR;
    }

    return PLAY_CONTROLLER_OK;
}