#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "../../include/debug_log.h"

#include "player_controller.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/player_dao_sqlite.h"

PlayerControllerStatus player_get_public_info(char* nickname, PlayerDTO** out_dto, int *out_count) {

    // Check if there's a player with this nickname
    Player retrievedPlayer;
    if (player_find_one_by_nickname(nickname, &retrievedPlayer) == PLAYER_CONTROLLER_NOT_FOUND) {
        *out_dto = NULL;
        *out_count = 0;
        return PLAYER_CONTROLLER_NOT_FOUND;
    }

    PlayerDTO *dynamicDTO = malloc(sizeof(PlayerDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return PLAYER_CONTROLLER_INTERNAL_ERROR;
    }

    map_player_to_dto(&retrievedPlayer, &(*dynamicDTO));
    
    *out_dto = dynamicDTO;
    *out_count = 1;
    
    return PLAYER_CONTROLLER_OK;
}

PlayerControllerStatus player_signup(char* nickname, char* email, char* password, int64_t* out_id_player) {

    // Input validation
    if (strlen(nickname) >= NICKNAME_MAX ||
        strlen(email) >= MAIL_MAX ||
        strlen(password) >= PASSWORD_MAX)
        return PLAYER_CONTROLLER_INVALID_INPUT;

    // Check if there's already a player with this nickname
    Player retrievedPlayer;
    if (player_find_one_by_nickname(nickname, &retrievedPlayer) != PLAYER_CONTROLLER_NOT_FOUND) {
        return PLAYER_CONTROLLER_STATE_VIOLATION;
    }

    // Build player to signup
    Player playerToSignup = {
        .current_streak = 0,
        .max_streak = 0,
        .registration_date = time(NULL)
    };
    strcpy(playerToSignup.nickname, nickname);
    strcpy(playerToSignup.email, email);
    strcpy(playerToSignup.password, password);

    // Create player
    PlayerControllerStatus status = player_create(&playerToSignup);
    if (status != PLAYER_CONTROLLER_OK)
        return status;

    *out_id_player = playerToSignup.id_player;

    return PLAYER_CONTROLLER_OK;
}

PlayerControllerStatus player_signin(char* nickname, char* password, bool* signedIn, int64_t* out_id_player) {

    *signedIn = false;

    // Check if there's a player with this nickname
    Player retrievedPlayer;
    if (player_find_one_by_nickname(nickname, &retrievedPlayer) == PLAYER_CONTROLLER_NOT_FOUND) {
        return PLAYER_CONTROLLER_INVALID_INPUT;
    }

    if (strcmp(retrievedPlayer.password, password) == 0)
        *signedIn = true;

    *out_id_player = retrievedPlayer.id_player;

    return PLAYER_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char* return_player_controller_status_to_string(PlayerControllerStatus status) {
    switch (status) {
        case PLAYER_CONTROLLER_OK:               return "PLAYER_CONTROLLER_OK";
        case PLAYER_CONTROLLER_INVALID_INPUT:    return "PLAYER_CONTROLLER_INVALID_INPUT";
        case PLAYER_CONTROLLER_NOT_FOUND:        return "PLAYER_CONTROLLER_NOT_FOUND";
        case PLAYER_CONTROLLER_STATE_VIOLATION:  return "PLAYER_CONTROLLER_STATE_VIOLATION";
        case PLAYER_CONTROLLER_DATABASE_ERROR:   return "PLAYER_CONTROLLER_DATABASE_ERROR";
        case PLAYER_CONTROLLER_CONFLICT:         return "PLAYER_CONTROLLER_CONFLICT";
        // case PLAYER_CONTROLLER_FORBIDDEN:        return "PLAYER_CONTROLLER_FORBIDDEN";
        case PLAYER_CONTROLLER_INTERNAL_ERROR:   return "PLAYER_CONTROLLER_INTERNAL_ERROR";
        default:                                return "PLAYER_CONTROLLER_UNKNOWN";
    }
}

// Create
PlayerControllerStatus player_create(Player* playerToCreate) {

    sqlite3* db = db_open();
    PlayerDaoStatus status = insert_player(db, playerToCreate);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Read all
PlayerControllerStatus player_find_all(Player** retrievedPlayerArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    PlayerDaoStatus status = get_all_players(db, retrievedPlayerArray, retrievedObjectCount);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Read one
PlayerControllerStatus player_find_one(int64_t id_player, Player* retrievedPlayer) {
    sqlite3* db = db_open();
    PlayerDaoStatus status = get_player_by_id(db, id_player, retrievedPlayer);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return status == PLAYER_DAO_NOT_FOUND ? PLAYER_CONTROLLER_NOT_FOUND : PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Update
PlayerControllerStatus player_update(Player* updatedPlayer) {
    sqlite3* db = db_open();
    PlayerDaoStatus status = update_player_by_id(db, updatedPlayer);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Delete
PlayerControllerStatus player_delete(int64_t id_player) {
    sqlite3* db = db_open();
    PlayerDaoStatus status = delete_player_by_id(db, id_player);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return status == PLAYER_DAO_NOT_FOUND ? PLAYER_CONTROLLER_NOT_FOUND : PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}

// Read one (by nickname)
PlayerControllerStatus player_find_one_by_nickname(const char *nickname, Player* retrievedPlayer) {
    sqlite3* db = db_open();
    PlayerDaoStatus status = get_player_by_nickname(db, nickname, retrievedPlayer);
    db_close(db);
    if (status != PLAYER_DAO_OK) {
        LOG_WARN("%s\n", return_player_dao_status_to_string(status));
        return status == PLAYER_DAO_NOT_FOUND ? PLAYER_CONTROLLER_NOT_FOUND : PLAYER_CONTROLLER_DATABASE_ERROR;
    }

    return PLAYER_CONTROLLER_OK;
}