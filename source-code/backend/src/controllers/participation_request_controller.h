#include <stdbool.h>

#include "../entities/participation_request_entity.h"

typedef enum {
    PARTICIPATION_REQUEST_CONTROLLER_OK = 0,
    //PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT,
    PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND,
    //PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION,
    PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR,
    //PARTICIPATION_REQUEST_CONTROLLER_CONFLICT,
    //PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN,
    //PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR
} ParticipationRequestControllerStatus;

// ===================== CRUD Operations =====================

ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participation_requestToCreate);
ParticipationRequestControllerStatus participation_request_find_all(ParticipationRequest** retrievedParticipationRequestArray, int* retrievedObjectCount);
ParticipationRequestControllerStatus participation_request_find_one(int id_participation_request, ParticipationRequest* retrievedParticipationRequest);
ParticipationRequestControllerStatus participation_request_update(ParticipationRequest* updatedParticipationRequest);
ParticipationRequestControllerStatus participation_request_delete(int id_participation_request);