#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <stdbool.h>

#include "../entities/player_entity.h"
#include "../dto/player_dto.h"

typedef enum {
    PLAYER_CONTROLLER_OK = 0,
    PLAYER_CONTROLLER_INVALID_INPUT,
    PLAYER_CONTROLLER_NOT_FOUND,
    PLAYER_CONTROLLER_STATE_VIOLATION,
    PLAYER_CONTROLLER_DATABASE_ERROR,
    PLAYER_CONTROLLER_CONFLICT,
    // PLAYER_CONTROLLER_FORBIDDEN,
    PLAYER_CONTROLLER_INTERNAL_ERROR
} PlayerControllerStatus;


PlayerControllerStatus player_get_public_info(char *nickname, PlayerDTO** out_dto, int *out_count);
PlayerControllerStatus player_signup(char *nickname, char *email, char *password, int64_t* out_id_player);
PlayerControllerStatus player_signin(char *nickname, char *password, bool* signedIn, int64_t* out_id_player);

// ===================== CRUD Operations =====================

PlayerControllerStatus player_create(Player* playerToCreate);
PlayerControllerStatus player_find_all(Player** retrievedPlayerArray, int* retrievedObjectCount);
PlayerControllerStatus player_find_one(int64_t id_player, Player* retrievedPlayer);
PlayerControllerStatus player_update(Player* updatedPlayer);
PlayerControllerStatus player_delete(int64_t id_player);

PlayerControllerStatus player_find_one_by_nickname(const char *nickname, Player* retrievedPlayer);

// Funzione di utilità per messaggi di errore
const char *return_player_controller_status_to_string(PlayerControllerStatus status);

#endif