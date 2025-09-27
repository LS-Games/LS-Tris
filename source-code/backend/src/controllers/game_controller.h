#include <stdbool.h>

#include "../entities/game_entity.h"
#include "../dto/game_dto.h"
#include "../db/sqlite/dto/game_join_player.h"

typedef enum {
    GAME_CONTROLLER_OK = 0,
    GAME_CONTROLLER_INVALID_INPUT,
    GAME_CONTROLLER_NOT_FOUND,
    //GAME_CONTROLLER_STATE_VIOLATION,
    GAME_CONTROLLER_DATABASE_ERROR,
    //GAME_CONTROLLER_CONFLICT,
    //GAME_CONTROLLER_FORBIDDEN,
    GAME_CONTROLLER_INTERNAL_ERROR
} GameControllerStatus;


GameControllerStatus game_start(int64_t id_creator);
GameControllerStatus games_get_public_info(GameDTO **out_dtos);

// ===================== CRUD Operations =====================

GameControllerStatus game_create(Game* gameToCreate);
GameControllerStatus game_find_all(Game** retrievedGameArray, int* retrievedObjectCount);
GameControllerStatus game_find_one(int id_game, Game* retrievedGame);
GameControllerStatus game_update(Game* updatedGame);
GameControllerStatus game_delete(int id_game);

GameControllerStatus game_find_all_with_player_info(GameWithPlayerNickname** retrievedGameArray, int* retrievedObjectCount);

// Funzione di utilità per messaggi di errore
const char* return_game_controller_status_to_string(GameControllerStatus status);