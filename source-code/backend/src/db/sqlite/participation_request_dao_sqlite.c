#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "participation_request_dao_sqlite.h"

const char* return_participation_request_status_to_string(ParticipationRequestReturnStatus status) {
    switch (status) {
        case PARTICIPATION_REQUEST_OK:             return "PARTICIPATION_REQUEST_OK";
        case PARTICIPATION_REQUEST_INVALID_INPUT:  return "PARTICIPATION_REQUEST_INVALID_INPUT";
        case PARTICIPATION_REQUEST_SQL_ERROR:      return "PARTICIPATION_REQUEST_SQL_ERROR";
        case PARTICIPATION_REQUEST_NOT_FOUND:      return "PARTICIPATION_REQUEST_NOT_FOUND";
        case PARTICIPATION_REQUEST_MALLOC_ERROR:   return  "PARTICIPATION_REQUEST_MALLOC_ERROR";
        default:                                   return "PARTICIPATION_REQUEST_UNKNOWN";
    }
}

ParticipationRequestReturnStatus get_participation_request_by_id(sqlite3 *db, int id_request, ParticipationRequest *out) {

    if(db == NULL || id_request <= 0 || out == NULL) {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_request, id_player, id_game, created_at, state"
        " FROM Participation_request WHERE id_request = ?1";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int(st , 1, id_request);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_request = sqlite3_column_int(st,0);
        out->id_player = sqlite3_column_int(st,1);
        out->id_game = sqlite3_column_int(st,2);
        const unsigned char *created_at = sqlite3_column_text(st, 3);
        const unsigned char *state = sqlite3_column_text(st,4);

        if (created_at) {
            strcpy(out->created_at, (const char*) created_at);
        } else {
            out->created_at[0] = '\0';
        }

        if (state) {
            out->state = string_to_request_participation_status((const char*) state);
        } else {
            out->state = REQUEST_STATUS_INVALID;
        }

        sqlite3_finalize(st); 
        return PARTICIPATION_REQUEST_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_NOT_FOUND;

    } else goto step_fail; 

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PARTICIPATION_REQUEST_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_SQL_ERROR;
}

ParticipationRequestReturnStatus get_all_participation_requests(sqlite3 *db, ParticipationRequest **out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_request, id_player, id_game, created_at, state FROM Participation_request"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    ParticipationRequest *p_request_array = malloc(sizeof(ParticipationRequest) * cap);

    if (!p_request_array) {
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            ParticipationRequest *tmp = realloc(p_request_array, sizeof(ParticipationRequest) * new_cap); 

            if(!tmp) {
                free(p_request_array);
                sqlite3_finalize(st);
                return PARTICIPATION_REQUEST_MALLOC_ERROR;
            }

            p_request_array = tmp; 
            cap = new_cap;
        }

        ParticipationRequest p_rqst;

        p_rqst.id_request = sqlite3_column_int(st,0);
        p_rqst.id_player = sqlite3_column_int(st,1);
        p_rqst.id_game = sqlite3_column_int(st,2);
        const unsigned char *created_at = sqlite3_column_text(st, 3);
        const unsigned char *state = sqlite3_column_text(st, 4);

        if (created_at) {
            strcpy(p_rqst.created_at, (const char*) created_at);
        } else {
            p_rqst.created_at[0] = '\0';
        }

        if (state) {
            p_rqst.state = string_to_request_participation_status((const char*) state);
        } else {
            p_rqst.state = REQUEST_STATUS_INVALID;
        }

        p_request_array[count++] = p_rqst;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(p_request_array);
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_SQL_ERROR;
    }

    *out_array = p_request_array;  
    *out_count = count;

    sqlite3_finalize(st);

    return PARTICIPATION_REQUEST_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PARTICIPATION_REQUEST_SQL_ERROR;
}

ParticipationRequestReturnStatus update_participation_request_by_id(sqlite3 *db, const ParticipationRequest *upd_participation_request) {

    if (db == NULL || upd_participation_request == NULL || upd_participation_request->id_request <= 0) {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    ParticipationRequest original_p_rquest;
    ParticipationRequestReturnStatus p_request_status = get_participation_request_by_id(db, upd_participation_request->id_request, &original_p_rquest);

    if (p_request_status != PARTICIPATION_REQUEST_OK) {
        return p_request_status;
    }

    UpdateParticipationRequestFlags flags = 0; 

    if(original_p_rquest.id_game != upd_participation_request->id_game) {
        flags |= UPDATE_REQUEST_ID_GAME;
    }


    if(original_p_rquest.id_player != upd_participation_request->id_player) {
        flags |= UPDATE_REQUEST_ID_PLAYER;
    }

    if(strcmp(original_p_rquest.created_at, upd_participation_request->created_at) != 0) {
        flags |= UPDATE_REQUEST_CREATED_AT;
    }

    if(original_p_rquest.state != upd_participation_request->state) {  
        flags |= UPDATE_REQUEST_STATE;
    }


    if (flags == 0) {
        return PARTICIPATION_REQUEST_NOT_MODIFIED;
    }

    char query[512] = "UPDATE Participation_request SET ";

    sqlite3_stmt *st = NULL;

    bool first = true; 

    if (flags & UPDATE_REQUEST_ID_GAME) { 
        
        if (!first) strcat(query, ", "); 
        strcat(query, "id_game = ?"); 
        first = false;
    }

    if(flags & UPDATE_REQUEST_ID_PLAYER) {
        if (!first) strcat(query, ", ");
        strcat(query, "id_player = ?");
        first = false;
    }

    if (flags & UPDATE_REQUEST_CREATED_AT) {
        if (!first) strcat(query, ", ");
        strcat(query, "created_at = ?");
        first = false;
    }

    if (flags & UPDATE_REQUEST_STATE) {
        if (!first) strcat(query, ", ");
        strcat(query, "state = ?");
        first = false;
    }

    strcat(query, " WHERE id_request = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    if (flags & UPDATE_REQUEST_ID_GAME) {
        rc = sqlite3_bind_int(st, param_index++, upd_participation_request->id_game);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_REQUEST_ID_PLAYER) {
        rc = sqlite3_bind_int(st, param_index++, upd_participation_request->id_player);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_REQUEST_CREATED_AT) {
        rc = sqlite3_bind_text(st, param_index++, upd_participation_request->created_at, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_REQUEST_STATE) {
        rc = sqlite3_bind_text(st, param_index++, request_participation_status_to_string(upd_participation_request->state), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    rc = sqlite3_bind_int(st, param_index, upd_participation_request->id_request);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return PARTICIPATION_REQUEST_NOT_MODIFIED;
    
    return PARTICIPATION_REQUEST_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PARTICIPATION_REQUEST_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PARTICIPATION_REQUEST_SQL_ERROR;
}

ParticipationRequestReturnStatus delete_participation_request_by_id(sqlite3 *db, int id_request) {

    if (db == NULL || id_request <= 0) {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Participation_request WHERE id_request = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int(stmt, 1, id_request);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return PARTICIPATION_REQUEST_NOT_FOUND;

    return PARTICIPATION_REQUEST_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PARTICIPATION_REQUEST_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PARTICIPATION_REQUEST_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PARTICIPATION_REQUEST_SQL_ERROR;
}

ParticipationRequestReturnStatus insert_participation_request(sqlite3 *db, const ParticipationRequest *in_request, sqlite3_int64 *out_id) {

    if (db == NULL || in_request == NULL) {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    if (in_request->id_game <= 0 || in_request->id_player <= 0 || in_request->created_at[0] == '\0') {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    if (in_request->state < PENDING || in_request->state > REJECTED) {
        return PARTICIPATION_REQUEST_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;
    const char *query =
        "INSERT INTO Participation_request (id_player, id_game, created_at, state) "
        "VALUES (?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int p = 1;

    rc = sqlite3_bind_int(stmt, p++, in_request->id_player);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(stmt, p++, in_request->id_game);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_text(stmt, p++, in_request->created_at, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    const char *state_str = request_participation_status_to_string(in_request->state);
    if (!state_str) { 
        sqlite3_finalize(stmt); 
        return PARTICIPATION_REQUEST_INVALID_INPUT; 
    }

    rc = sqlite3_bind_text(stmt, p++, state_str, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) goto step_fail;

    sqlite3_int64 id = sqlite3_last_insert_rowid(db);

    if(out_id) { *out_id = id; }

    sqlite3_finalize(stmt);
    return PARTICIPATION_REQUEST_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PARTICIPATION_REQUEST_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PARTICIPATION_REQUEST_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PARTICIPATION_REQUEST_SQL_ERROR;
}

