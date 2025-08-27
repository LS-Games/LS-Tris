#ifndef PARTICIPATION_REQUEST_DAO_SQLITE_H
#define PARTICIPATION_REQUEST_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/participation_request_entity.h"

typedef enum {
    PARTICIPATION_REQUEST_OK = 0,
    PARTICIPATION_REQUEST_NOT_FOUND,
    PARTICIPATION_REQUEST_SQL_ERROR,
    PARTICIPATION_REQUEST_INVALID_INPUT,
    PARTICIPATION_REQUEST_MALLOC_ERROR,
    PARTICIPATION_REQUEST_NOT_MODIFIED
} ParticipationRequestReturnStatus;

typedef enum {
    UPDATE_REQUEST_ID_PLAYER        = 1 << 0,  
    UPDATE_REQUEST_ID_GAME          = 1 << 1,  
    UPDATE_REQUEST_CREATED_AT       = 1 << 2,
    UPDATE_REQUEST_STATE            = 1 << 3
} UpdateParticipationRequestFlags;


// Funzioni CRUD concrete
ParticipationRequestReturnStatus get_participation_request_by_id(sqlite3 *db, int id_request, ParticipationRequest *out);  
ParticipationRequestReturnStatus get_all_participation_requests(sqlite3 *db, ParticipationRequest** out_array, int *out_count);
ParticipationRequestReturnStatus update_participation_request_by_id(sqlite3 *db, const ParticipationRequest *upd_participation_request);
ParticipationRequestReturnStatus delete_participation_request_by_id(sqlite3 *db, int id_request);
ParticipationRequestReturnStatus insert_participation_request(sqlite3 *db, const ParticipationRequest *in_request);

// Funzione di utilità per messaggi di errore
const char* return_participation_request_status_to_string(ParticipationRequestReturnStatus status);

#endif