#include "../../include/debug_log.h"

#include "participation_request_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/participation_request_dao_sqlite.h"

// ===================== CRUD Operations =====================

const char* return_participation_request_controller_status_to_string(ParticipationRequestControllerStatus status) {
    switch (status) {
        case PARTICIPATION_REQUEST_CONTROLLER_OK:               return "PARTICIPATION_REQUEST_CONTROLLER_OK";
        // case PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT:    return "PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT";
        case PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND:        return "PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND";
        // case PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION:  return "PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION";
        case PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR";
        // case PARTICIPATION_REQUEST_CONTROLLER_CONFLICT:         return "PARTICIPATION_REQUEST_CONTROLLER_CONFLICT";
        // case PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN:        return "PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN";
        // case PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR";
        default:                                return "PARTICIPATION_REQUEST_CONTROLLER_UNKNOWN";
    }
}

// Create
ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participationRequestToCreate) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = insert_participation_request(db, participationRequestToCreate);
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
    ParticipationRequestReturnStatus status = get_all_participation_requests(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read one
ParticipationRequestControllerStatus participation_request_find_one(int id_participation_request, ParticipationRequest* retrievedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = get_participation_request_by_id(db, id_participation_request, retrievedParticipationRequest);
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
    ParticipationRequestReturnStatus status = update_participation_request_by_id(db, updatedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Delete
ParticipationRequestControllerStatus participation_request_delete(int id_participation_request) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = delete_participation_request_by_id(db, id_participation_request);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}