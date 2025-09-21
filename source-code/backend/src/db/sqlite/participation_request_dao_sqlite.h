#ifndef PARTICIPATION_REQUEST_DAO_SQLITE_H
#define PARTICIPATION_REQUEST_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/participation_request_entity.h"
#include "../../db/sqlite/dto/participation_request_join_player.h"

typedef enum {
    PARTICIPATION_DAO_REQUEST_OK = 0,
    PARTICIPATION_DAO_REQUEST_NOT_FOUND,
    PARTICIPATION_DAO_REQUEST_SQL_ERROR,
    PARTICIPATION_DAO_REQUEST_INVALID_INPUT,
    PARTICIPATION_DAO_REQUEST_MALLOC_ERROR,
    PARTICIPATION_DAO_REQUEST_NOT_MODIFIED
} ParticipationRequestReturnStatus;

typedef enum {
    UPDATE_REQUEST_ID_PLAYER        = 1 << 0,  
    UPDATE_REQUEST_ID_GAME          = 1 << 1,  
    UPDATE_REQUEST_CREATED_AT       = 1 << 2,
    UPDATE_REQUEST_STATE            = 1 << 3
} UpdateParticipationRequestFlags;


// Funzioni CRUD concrete
ParticipationRequestReturnStatus get_participation_request_by_id(sqlite3 *db, int64_t id_request, ParticipationRequest *out);  
ParticipationRequestReturnStatus get_all_participation_requests(sqlite3 *db, ParticipationRequest** out_array, int *out_count);
ParticipationRequestReturnStatus update_participation_request_by_id(sqlite3 *db, const ParticipationRequest *upd_participation_request);
ParticipationRequestReturnStatus delete_participation_request_by_id(sqlite3 *db, int64_t id_request);
ParticipationRequestReturnStatus insert_participation_request(sqlite3 *db, ParticipationRequest *in_out_request);

ParticipationRequestReturnStatus get_all_participation_requests_with_player_info(sqlite3 *db, ParticipationRequestWithPlayerNickname **out_array, int *out_count);

// Funzione di utilità per messaggi di errore
const char* return_participation_request_dao_status_to_string(ParticipationRequestReturnStatus status);

#endif