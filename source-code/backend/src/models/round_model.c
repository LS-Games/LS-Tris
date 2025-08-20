#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "round_model.h"

const char* round_status_to_string(RoundStatus state) {
    switch (state) {
        case ACTIVE_ROUND :               return "active";
        case PENDING_ROUND :              return "pending";
        case FINISHED_ROUND :             return "finished";
        default:                    return NULL;
    }
}

const char* return_round_status_to_string(RoundReturnStatus status) {
    switch (status) {
        case ROUND_OK:             return "ROUND_OK";
        case ROUND_INVALID_INPUT:  return "ROUND_INVALID_INPUT";
        case ROUND_SQL_ERROR:      return "ROUND_SQL_ERROR";
        case ROUND_NOT_FOUND:      return "ROUND_NOT_FOUND";
        default:                  return NULL;
    }
}

RoundStatus string_to_round_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "active") == 0) return ACTIVE_ROUND;
        if (strcmp(state_str, "pending") == 0) return PENDING_ROUND;
        if (strcmp(state_str, "finished") == 0) return FINISHED_ROUND;
    }

    return ROUND_STATUS_INVALID;
}

RoundReturnStatus get_round_by_id(sqlite3 *db, int id_round, Round *out) {

    if(db == NULL || id_round <= 0 || out == NULL) {
        return ROUND_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_round, id_game, state, duration "
        "FROM Round WHERE id_round = ?1";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int(st , 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_round = sqlite3_column_int(st, 0); 
        out->id_game = sqlite3_column_int(st, 1); 
        const unsigned char *state = sqlite3_column_text(st, 2); 
        out->duration = sqlite3_column_int64(st, 3); 

        if (state) {
            out->state = string_to_round_status((const char*) state);
        } else {
            out->state = ROUND_STATUS_INVALID;
        }

        sqlite3_finalize(st); 
        return ROUND_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return ROUND_NOT_FOUND;

    } else goto step_fail;
    
    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_SQL_ERROR;
}

RoundReturnStatus get_all_rounds(sqlite3 *db, Round** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return ROUND_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_round, id_game, state, duration FROM Round"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    Round *rounds_array = malloc(sizeof(Round) * cap);

    if (!rounds_array) {
        sqlite3_finalize(st);
        return ROUND_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Round *tmp = realloc(rounds_array, sizeof(Round)* new_cap); 

            if(!tmp) {
                free(rounds_array);
                sqlite3_finalize(st);
                return ROUND_MALLOC_ERROR;
            }

            rounds_array = tmp; 
            cap = new_cap;
        }

        Round r;

        r.id_round = sqlite3_column_int(st,0);
        r.id_game = sqlite3_column_int(st, 1);
        const unsigned char *state = sqlite3_column_text(st, 2);
        r.duration = sqlite3_column_int64(st,3);

        if (state) {
            r.state = string_to_round_status((const char*) state);
        } else {
            r.state = ROUND_STATUS_INVALID;
        }

        rounds_array[count++] = r;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(rounds_array);
        sqlite3_finalize(st);
        return ROUND_SQL_ERROR;
    }

    *out_array = rounds_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return ROUND_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_SQL_ERROR;

}

RoundReturnStatus update_round_by_id(sqlite3 *db, const Round *upd_round) {

    if (db == NULL || upd_round == NULL || upd_round->id_round <= 0) {
        return ROUND_INVALID_INPUT;
    }

    Round original_round;
    RoundReturnStatus round_status = get_round_by_id(db, upd_round->id_round, &original_round);

    if (round_status != ROUND_OK) {
        return round_status;
    }

    UpdateRoundFlags flags = 0; 

    if(original_round.id_game != upd_round->id_game) {
        flags |= UPDATE_ROUND_ID_GAME;
    }

    if(original_round.state != upd_round->state) {  
        flags |= UPDATE_ROUND_STATE;
    }

    if(original_round.duration != upd_round->duration) {
        flags |= UPDATE_ROUND_DURATION;
    }

    if (flags == 0) {
        return ROUND_NOT_MODIFIED;
    }

    char query[512] = "UPDATE Round SET ";

    sqlite3_stmt *st = NULL;

    bool first = true; 

    if (flags & UPDATE_ROUND_ID_GAME) { 
        if (!first) strcat(query, ", "); 
        strcat(query, "id_game = ?"); 
        first = false;
    }

    if(flags & UPDATE_ROUND_STATE) {
        if (!first) strcat(query, ", ");
        strcat(query, "state = ?");
        first = false;
    }

    if (flags & UPDATE_ROUND_DURATION) {
        if (!first) strcat(query, ", ");
        strcat(query, "duration = ?");
        first = false;
    }

    strcat(query, " WHERE id_round = ?");

    printf("\n\nQUERY\n\n: %s", query);

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    if (flags & UPDATE_ROUND_ID_GAME) {
        rc = sqlite3_bind_int(st, param_index++, upd_round->id_game);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_ROUND_STATE) {
        rc = sqlite3_bind_text(st, param_index++, round_status_to_string(upd_round->state), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_ROUND_DURATION) {
        rc = sqlite3_bind_int64(st, param_index++, upd_round->duration);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    rc = sqlite3_bind_int(st, param_index, upd_round->id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return ROUND_NOT_MODIFIED;
    
    return ROUND_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_SQL_ERROR;
}

RoundReturnStatus delete_round_by_id(sqlite3 *db, int id_round) {

    if (db == NULL || id_round <= 0) {
        return ROUND_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Round WHERE id_round = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int(stmt, 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return ROUND_NOT_FOUND;

    return ROUND_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_SQL_ERROR;
}

RoundReturnStatus insert_round(sqlite3 *db, const Round *in_round) {

    if (db == NULL || in_round == NULL) {
        return ROUND_INVALID_INPUT;
    }

    if (in_round->id_game <= 0 || in_round->duration < 0) {
        return ROUND_INVALID_INPUT;
    }

    if (in_round->state < ACTIVE_ROUND || in_round->state > FINISHED_ROUND) {
        return ROUND_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char* query = 
        "INSERT INTO Round (id_game, state, duration)"
        " VALUES ( ?1, ?2, ?3)";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    rc = sqlite3_bind_int(stmt, param_index++, in_round->id_game);
    if (rc != SQLITE_OK) goto bind_fail;

    const char* r_st = round_status_to_string(in_round->state);
    if(!r_st) {
        sqlite3_finalize(stmt);
        return ROUND_INVALID_INPUT;
    }

    rc = sqlite3_bind_text(stmt, param_index++, r_st, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, param_index++, in_round->duration);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_DONE) goto step_fail;

    if (sqlite3_changes(db) == 0) return ROUND_NOT_MODIFIED;

    sqlite3_finalize(stmt);
    return ROUND_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_SQL_ERROR;
}
