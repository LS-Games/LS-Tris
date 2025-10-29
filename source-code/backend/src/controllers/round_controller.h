#ifndef ROUND_CONTROLLER_H
#define ROUND_CONTROLLER_H

#include <stdbool.h>

#include "../entities/round_entity.h"
#include "../dto/round_dto.h"

typedef enum {
    ROUND_CONTROLLER_OK = 0,
    ROUND_CONTROLLER_INVALID_INPUT,
    ROUND_CONTROLLER_NOT_FOUND,
    ROUND_CONTROLLER_STATE_VIOLATION,
    ROUND_CONTROLLER_DATABASE_ERROR,
    // ROUND_CONTROLLER_CONFLICT,
    ROUND_CONTROLLER_FORBIDDEN,
    ROUND_CONTROLLER_INTERNAL_ERROR
} RoundControllerStatus;


RoundControllerStatus round_get_public_info(int64_t id_round, RoundDTO **out_dto, int *out_count);
RoundControllerStatus round_make_move(int64_t id_round, int64_t id_player, int row, int col, int64_t* out_id_round);
RoundControllerStatus round_end(int64_t id_round, int64_t* out_id_round);

// ===================== Controllers Helper Functions =====================

RoundControllerStatus round_start(int64_t id_game, int64_t id_player1, int64_t id_player2, int64_t duration);

// ===================== CRUD Operations =====================

RoundControllerStatus round_create(Round* roundToCreate);
RoundControllerStatus round_find_all(Round **retrievedRoundArray, int* retrievedObjectCount);
RoundControllerStatus round_find_one(int64_t id_round, Round* retrievedRound);
RoundControllerStatus round_update(Round* updatedRound);
RoundControllerStatus round_delete(int64_t id_round);

// Funzione di utilità per messaggi di errore
const char *return_round_controller_status_to_string(RoundControllerStatus status);

#endif