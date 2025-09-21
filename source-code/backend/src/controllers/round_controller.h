#include <stdbool.h>

#include "../entities/round_entity.h"

typedef enum {
    ROUND_CONTROLLER_OK = 0,
    ROUND_CONTROLLER_INVALID_INPUT,
    ROUND_CONTROLLER_NOT_FOUND,
    ROUND_CONTROLLER_STATE_VIOLATION,
    ROUND_CONTROLLER_DATABASE_ERROR,
    //ROUND_CONTROLLER_CONFLICT,
    ROUND_CONTROLLER_FORBIDDEN,
    ROUND_CONTROLLER_INTERNAL_ERROR
} RoundControllerStatus;


RoundControllerStatus round_start(int id_game, int64_t duration);
RoundControllerStatus round_make_move(int id_round, int id_player, int row, int col);
RoundControllerStatus round_end(int id_round);

// ===================== CRUD Operations =====================

RoundControllerStatus round_create(Round* roundToCreate);
RoundControllerStatus round_find_all(Round** retrievedRoundArray, int* retrievedObjectCount);
RoundControllerStatus round_find_one(int id_round, Round* retrievedRound);
RoundControllerStatus round_update(Round* updatedRound);
RoundControllerStatus round_delete(int id_round);

// Funzione di utilità per messaggi di errore
const char* return_round_controller_status_to_string(RoundControllerStatus status);