#ifndef PLAY_CONTROLLER_H
#define PLAY_CONTROLLER_H

#include <stdbool.h>

#include "../entities/play_entity.h"
#include "../dto/play_dto.h"
#include "../dao/dto/play_join_player.h"

typedef enum {
    PLAY_CONTROLLER_OK = 0,
    PLAY_CONTROLLER_INVALID_INPUT,
    PLAY_CONTROLLER_NOT_FOUND,
    // PLAY_CONTROLLER_STATE_VIOLATION,
    PLAY_CONTROLLER_DATABASE_ERROR,
    // PLAY_CONTROLLER_CONFLICT,
    // PLAY_CONTROLLER_FORBIDDEN,
    PLAY_CONTROLLER_INTERNAL_ERROR
} PlayControllerStatus;


PlayControllerStatus plays_get_public_info(int64_t id_player, int64_t id_round, PlayDTO** out_dtos);
PlayControllerStatus play_add_round_plays(int64_t id_round, int64_t id_player_1, int64_t id_player_2);
PlayControllerStatus play_set_round_plays_result(int64_t id_round, PlayResult result, int winner);
PlayControllerStatus play_retrieve_round_current_player_number(int64_t id_round, int64_t id_currentPlayer, int* out_player_number);
PlayControllerStatus play_find_round_winner(int64_t id_round, int64_t *out_id_playerWinner);

// ===================== CRUD Operations =====================

PlayControllerStatus play_create(Play* playToCreate);
PlayControllerStatus play_find_all(Play** retrievedPlayArray, int* retrievedObjectCount);
PlayControllerStatus play_find_one(int64_t id_player, int64_t id_round, Play* retrievedPlay);
PlayControllerStatus play_update(Play* updatedPlay);
PlayControllerStatus play_delete(int64_t id_player, int64_t id_round);

PlayControllerStatus play_find_all_by_id_round(Play** retrievedPlayArray, int64_t id_round, int* retrievedObjectCount);
PlayControllerStatus play_find_all_with_player_info(PlayWithPlayerNickname** retrievedPlayArray, int* retrievedObjectCount);

// Funzione di utilità per messaggi di errore
const char* return_play_controller_status_to_string(PlayControllerStatus status);

#endif