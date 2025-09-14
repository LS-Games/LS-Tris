#include <string.h>
#include <time.h>

#include "../../include/debug_log.h"

#include "player_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/player_dao_sqlite.h"

PlayerControllerStatus player_signup(char* nickname, char* email, char* password) {

    // Input validation
    if (strlen(nickname) >= NICKNAME_MAX ||
        strlen(email) >= MAIL_MAX ||
        strlen(password) >= PASSWORD_MAX)
        return PLAYER_CONTROLLER_INVALID_INPUT;

    Player player = {
        .current_streak = 0,
        .max_streak = 0,
        .registration_date = time(NULL)
    };
    strcpy(player.nickname, nickname);
    strcpy(player.email, email);
    strcpy(player.password, password);

    return player_create(&player);
}


// ===================== CRUD Operations =====================

// Create
PlayerControllerStatus player_create(Player* playerToCreate) {
    sqlite3* db = db_open();
    PlayerReturnStatus status = insert_player(db, playerToCreate);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Read all
PlayerControllerStatus player_find_all(Player** retrievedPlayerArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayerReturnStatus status = get_all_players(db, retrievedPlayerArray, retrievedObjectCount);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Read one
PlayerControllerStatus player_find_one(int id_player, Player* retrievedPlayer) {
    sqlite3* db = db_open();
    PlayerReturnStatus status = get_player_by_id(db, id_player, retrievedPlayer);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_status_to_string(status));
        return status == PLAYER_DAO_NOT_FOUND ? PLAYER_CONTROLLER_NOT_FOUND : PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Update
PlayerControllerStatus player_update(Player* updatedPlayer) {
    sqlite3* db = db_open();
    PlayerReturnStatus status = update_player_by_id(db, updatedPlayer);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Delete
PlayerControllerStatus player_delete(int id_player) {
    sqlite3* db = db_open();
    PlayerReturnStatus status = delete_player_by_id(db, id_player);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_status_to_string(status));
        return status == PLAYER_DAO_NOT_FOUND ? PLAYER_CONTROLLER_NOT_FOUND : PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}