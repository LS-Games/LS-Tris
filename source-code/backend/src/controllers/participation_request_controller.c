#include "../../include/debug_log.h"

#include "participation_request_controller.h"
#include "../db/sqlite/db_connection_sqlite.h"
#include "../db/sqlite/participation_request_dao_sqlite.h"

// ===================== CRUD Operations =====================

// Create
ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participationRequestToCreate) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = insert_participation_request(db, participationRequestToCreate);
    db_close(db);
    if (status != PARTICIPATION_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }
    LOG_WARN("%p\n", (void *)participationRequestToCreate);
    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all
ParticipationRequestControllerStatus participation_request_find_all(ParticipationRequest** retrievedParticipationRequestArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = get_all_participation_requests(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read one
ParticipationRequestControllerStatus participation_request_find_one(int id_participation_request, ParticipationRequest* retrievedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = get_participation_request_by_id(db, id_participation_request, retrievedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_status_to_string(status));
        return status == PARTICIPATION_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Update
ParticipationRequestControllerStatus participation_request_update(ParticipationRequest* updatedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = update_participation_request_by_id(db, updatedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Delete
ParticipationRequestControllerStatus participation_request_delete(int id_participation_request) {
    sqlite3* db = db_open();
    ParticipationRequestReturnStatus status = delete_participation_request_by_id(db, id_participation_request);
    db_close(db);
    if (status != PARTICIPATION_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_status_to_string(status));
        return status == PARTICIPATION_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}