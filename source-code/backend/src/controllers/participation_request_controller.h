#ifndef PARTICIPATION_REQUEST_CONTROLLER_H
#define PARTICIPATION_REQUEST_CONTROLLER_H

#include <stdbool.h>

#include "../entities/participation_request_entity.h"
#include "../dto/participation_request_dto.h"
#include "../dao/dto/participation_request_join_player.h"

typedef enum {
    PARTICIPATION_REQUEST_CONTROLLER_OK = 0,
    PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT,
    PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND,
    // PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION,
    PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR,
    // PARTICIPATION_REQUEST_CONTROLLER_CONFLICT,
    // PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN,
    PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR
} ParticipationRequestControllerStatus;


ParticipationRequestControllerStatus participation_requests_get_public_info_by_state(char* state, int64_t id_game, ParticipationRequestDTO** out_dtos);
ParticipationRequestControllerStatus participation_request_send(int64_t id_game, int64_t id_player);
ParticipationRequestControllerStatus participation_request_change_state(int64_t id_participation_request, RequestStatus newStatus);
ParticipationRequestControllerStatus participation_request_cancel(int64_t id_participation_request);

// ===================== CRUD Operations =====================

ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participation_requestToCreate);
ParticipationRequestControllerStatus participation_request_find_all(ParticipationRequest** retrievedParticipationRequestArray, int* retrievedObjectCount);
ParticipationRequestControllerStatus participation_request_find_one(int64_t id_participation_request, ParticipationRequest* retrievedParticipationRequest);
ParticipationRequestControllerStatus participation_request_update(ParticipationRequest* updatedParticipationRequest);
ParticipationRequestControllerStatus participation_request_delete(int64_t id_participation_request);

ParticipationRequestControllerStatus participation_request_find_all_with_player_info(ParticipationRequestWithPlayerNickname** retrievedParticipationRequestArray, int* retrievedObjectCount);
ParticipationRequestControllerStatus participation_request_find_all_pending_by_id_game(ParticipationRequest** retrievedParticipationRequestArray, int64_t id_game, int* retrievedObjectCount);

// Funzione di utilità per messaggi di errore
const char* return_participation_request_controller_status_to_string(ParticipationRequestControllerStatus status);

#endif