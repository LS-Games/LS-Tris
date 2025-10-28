#include <stdlib.h>
#include <string.h>

#include "../../include/debug_log.h"

#include "participation_request_controller.h"
#include "round_controller.h"
#include "game_controller.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/participation_request_dao_sqlite.h"

// ==================== Private functions ====================

static ParticipationRequestControllerStatus participation_request_accept_helper(int64_t id_gameToPlay, int64_t id_playerAccepted);
static ParticipationRequestControllerStatus participation_request_reject_all(ParticipationRequest* pendingRequestsToReject, int retrievedObjectCount);

// ===========================================================

// This function provides a query by `state` and `id_game`. 
// @param state Possible values are `pending`, `accepted`, `rejected` and `all` (no filter)
// @param id_game Possible values are all integer positive number and -1 (no filter)
ParticipationRequestControllerStatus participation_requests_get_public_info(char* state, int64_t id_game, ParticipationRequestDTO** out_dtos, int *out_count) {

    RequestStatus queryState = REQUEST_STATUS_INVALID;
    if (strcmp(state, "all") != 0)
        queryState = string_to_request_participation_status(state);
    if (queryState == REQUEST_STATUS_INVALID)
        return PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT;

    ParticipationRequestWithPlayerNickname* retrievedParticipationRequests;
    int retrievedObjectCount;
    if (participation_request_find_all_with_player_info(&retrievedParticipationRequests, &retrievedObjectCount) == PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND) {
        return PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT;
    }

    ParticipationRequestDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if ((strcmp(state, "all") == 0 || retrievedParticipationRequests[i].state == queryState) &&
            (id_game == -1 || retrievedParticipationRequests[i].id_game == id_game)) {

            ParticipationRequest participationRequest = {
                .id_request = retrievedParticipationRequests[i].id_request,
                .id_game = retrievedParticipationRequests[i].id_game,
                .created_at = retrievedParticipationRequests[i].created_at,
                .state = retrievedParticipationRequests[i].state
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(ParticipationRequestDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
            }

            map_participation_request_to_dto(&participationRequest, retrievedParticipationRequests[i].player_nickname, &(dynamicDTOs[filteredObjectCount]));
        
            filteredObjectCount = filteredObjectCount + 1;
        }
    }

    *out_dtos = dynamicDTOs;
    *out_count = filteredObjectCount;
    
    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

ParticipationRequestControllerStatus participation_request_send(int64_t id_game, int64_t id_player) {

    // Build participation request to send
    ParticipationRequest participationRequestToSend = {
        .id_game = id_game,
        .id_player = id_player,
        .created_at = time(NULL),
        .state = PENDING
    };

    // Create participation request
    return participation_request_create(&participationRequestToSend);
}

ParticipationRequestControllerStatus participation_request_change_state(int64_t id_participation_request, RequestStatus newStatus) {

    // Retrieve participation request to change state
    ParticipationRequest retrievedParticipationRequest;
    ParticipationRequestControllerStatus status = participation_request_find_one(id_participation_request, &retrievedParticipationRequest);
    if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return status;

    retrievedParticipationRequest.state = newStatus;

    // If the request is accepted, we should start a new round and reject all other requests
    if (retrievedParticipationRequest.state == ACCEPTED) {
        status = participation_request_accept_helper(retrievedParticipationRequest.id_game, retrievedParticipationRequest.id_player);
        if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return status;
    }

    return participation_request_update(&retrievedParticipationRequest);
}

static ParticipationRequestControllerStatus participation_request_accept_helper(int64_t id_gameToPlay, int64_t id_playerAccepted) {

    // Retrieve the participation request's game
    Game retrievedGame;
    GameControllerStatus gameStatus = game_find_one(id_gameToPlay, &retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    // Start the round
    RoundControllerStatus roundStatus = round_start(retrievedGame.id_game, retrievedGame.id_owner, id_playerAccepted, 500);
    if (roundStatus != ROUND_CONTROLLER_OK) {
        LOG_WARN("%s\n", return_round_controller_status_to_string(roundStatus));
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }

    // Update the game status
    retrievedGame.state = ACTIVE_GAME;
    gameStatus = game_update(&retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    // Reject pending requests
    ParticipationRequest* retrievedPendingRequests;
    int retrievedObjectCount;
    ParticipationRequestControllerStatus participationRequestStatus = participation_request_find_all_pending_by_id_game(&retrievedPendingRequests, retrievedGame.id_game, &retrievedObjectCount);
    if (participationRequestStatus != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return participationRequestStatus;

    return participation_request_reject_all(retrievedPendingRequests, retrievedObjectCount);
}

static ParticipationRequestControllerStatus participation_request_reject_all(ParticipationRequest* pendingRequestsToReject, int retrievedObjectCount) {
    for (int i = 0; i < retrievedObjectCount; i++) {
        pendingRequestsToReject[i].state = REJECTED;
        ParticipationRequestControllerStatus status = participation_request_update(&(pendingRequestsToReject[i]));
        if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return status;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

ParticipationRequestControllerStatus participation_request_cancel(int64_t id_participation_request) {
    return participation_request_delete(id_participation_request);
}

// ===================== CRUD Operations =====================

const char* return_participation_request_controller_status_to_string(ParticipationRequestControllerStatus status) {
    switch (status) {
        case PARTICIPATION_REQUEST_CONTROLLER_OK:               return "PARTICIPATION_REQUEST_CONTROLLER_OK";
        case PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT:    return "PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT";
        case PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND:        return "PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND";
        // case PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION:  return "PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION";
        case PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR";
        // case PARTICIPATION_REQUEST_CONTROLLER_CONFLICT:         return "PARTICIPATION_REQUEST_CONTROLLER_CONFLICT";
        // case PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN:        return "PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN";
        case PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR";
        default:                                return "PARTICIPATION_REQUEST_CONTROLLER_UNKNOWN";
    }
}

// Create
ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participationRequestToCreate) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = insert_participation_request(db, participationRequestToCreate);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all
ParticipationRequestControllerStatus participation_request_find_all(ParticipationRequest** retrievedParticipationRequestArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_participation_requests(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read one
ParticipationRequestControllerStatus participation_request_find_one(int64_t id_participation_request, ParticipationRequest* retrievedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_participation_request_by_id(db, id_participation_request, retrievedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Update
ParticipationRequestControllerStatus participation_request_update(ParticipationRequest* updatedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = update_participation_request_by_id(db, updatedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Delete
ParticipationRequestControllerStatus participation_request_delete(int64_t id_participation_request) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = delete_participation_request_by_id(db, id_participation_request);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all with player info
ParticipationRequestControllerStatus participation_request_find_all_with_player_info(ParticipationRequestWithPlayerNickname** retrievedParticipationRequestArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_participation_requests_with_player_info(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all (state="pending" by id_game)
ParticipationRequestControllerStatus participation_request_find_all_pending_by_id_game(ParticipationRequest** retrievedParticipationRequestArray, int64_t id_game, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_pending_participation_request_by_id_game(db, id_game, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}