#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <stdbool.h>

#include "../entities/game_entity.h"
#include "../dto/game_dto.h"
#include "../dao/dto/game_join_player.h"

typedef enum {
    GAME_CONTROLLER_OK = 0,
    GAME_CONTROLLER_INVALID_INPUT,
    GAME_CONTROLLER_NOT_FOUND,
    // GAME_CONTROLLER_STATE_VIOLATION,
    GAME_CONTROLLER_DATABASE_ERROR,
    // GAME_CONTROLLER_CONFLICT,
    GAME_CONTROLLER_FORBIDDEN,
    GAME_CONTROLLER_INTERNAL_ERROR
} GameControllerStatus;


GameControllerStatus games_get_public_info(char *status, GameDTO **out_dtos, int *out_count);
GameControllerStatus game_start(int64_t id_creator, int64_t* out_id_game);
GameControllerStatus game_end(int64_t id_game, int64_t id_owner, int64_t* out_id_game);
GameControllerStatus game_forfeit(int64_t id_game, int64_t id_leaver, int64_t* out_winner);
GameControllerStatus game_refuse_rematch(int64_t id_game, int64_t* out_id_game);
GameControllerStatus game_accept_rematch(int64_t id_game, int64_t id_playerAcceptingRematch, int64_t* out_id_game, int* out_waiting);
GameControllerStatus game_cancel(int64_t id_game, int64_t id_owner, int64_t* out_id_game);

// ===================== Controllers Helper Functions =====================

GameControllerStatus game_change_owner(int64_t id_game, int64_t id_newOwner);

// ===================== CRUD Operations =====================

GameControllerStatus game_create(Game* gameToCreate);
GameControllerStatus game_find_all(Game **retrievedGameArray, int* retrievedObjectCount);
GameControllerStatus game_find_one(int64_t id_game, Game* retrievedGame);
GameControllerStatus game_update(Game* updatedGame);
GameControllerStatus game_delete(int64_t id_game);

GameControllerStatus game_find_one_with_player_info(int64_t id_game, GameWithPlayerNickname* retrievedGame);
GameControllerStatus game_find_all_with_player_info(GameWithPlayerNickname **retrievedGameArray, int* retrievedObjectCount);

// Funzione di utilità per messaggi di errore
const char *return_game_controller_status_to_string(GameControllerStatus status);

#endif