#include <stdbool.h>

#include "../entities/player_entity.h"

typedef enum {
    PLAYER_CONTROLLER_OK = 0,
    PLAYER_CONTROLLER_INVALID_INPUT,
    PLAYER_CONTROLLER_NOT_FOUND,
    //PLAYER_CONTROLLER_STATE_VIOLATION,
    PLAYER_CONTROLLER_DATABASE_ERROR,
    //PLAYER_CONTROLLER_CONFLICT,
    //PLAYER_CONTROLLER_FORBIDDEN,
    //PLAYER_CONTROLLER_INTERNAL_ERROR
} PlayerControllerStatus;


PlayerControllerStatus player_signup(char* nickname, char* email, char* password);

// ===================== CRUD Operations =====================

PlayerControllerStatus player_create(Player* playerToCreate);
PlayerControllerStatus player_find_all(Player** retrievedPlayerArray, int* retrievedObjectCount);
PlayerControllerStatus player_find_one(int id_player, Player* retrievedPlayer);
PlayerControllerStatus player_update(Player* updatedPlayer);
PlayerControllerStatus player_delete(int id_player);