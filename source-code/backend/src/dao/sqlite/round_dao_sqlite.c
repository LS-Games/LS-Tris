#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../../include/debug_log.h"

#include "round_dao_sqlite.h"

const char *return_round_dao_status_to_string(RoundDaoStatus status) {
    switch (status) {
        case ROUND_DAO_OK:              return "ROUND_DAO_OK";
        case ROUND_DAO_INVALID_INPUT:   return "ROUND_DAO_INVALID_INPUT";
        case ROUND_DAO_SQL_ERROR:       return "ROUND_DAO_SQL_ERROR";
        case ROUND_DAO_NOT_FOUND:       return "ROUND_DAO_NOT_FOUND";
        default:                        return "ROUND_DAO_UNKNOWN";
    }
}

RoundDaoStatus get_round_by_id(sqlite3 *db, int64_t id_round, Round *out) {

    if(db == NULL || id_round <= 0 || out == NULL) {
        return ROUND_DAO_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_round, id_game, state, duration, board "
        "FROM Round WHERE id_round = ?1";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(st , 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_round = sqlite3_column_int64(st, 0); 
        out->id_game = sqlite3_column_int64(st, 1); 
        const unsigned char *state = sqlite3_column_text(st, 2); 
        out->duration = sqlite3_column_int64(st, 3); 
        const unsigned char *board = sqlite3_column_text(st, 4); 

        if (state) {
            out->state = string_to_round_status((const char*) state);
        } else {
            out->state = ROUND_STATUS_INVALID;
        }

        if (board) {
            strcpy(out->board, (const char*) board);
        } else {
            out->board[0] = '\0';
        }

        sqlite3_finalize(st); 
        return ROUND_DAO_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return ROUND_DAO_NOT_FOUND;

    } else goto step_fail;
    
    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_DAO_SQL_ERROR;
}

RoundDaoStatus get_all_rounds(sqlite3 *db, Round** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return ROUND_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_round, id_game, state, duration, board FROM Round"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    Round *rounds_array = malloc(sizeof(Round) * cap);

    if (!rounds_array) {
        sqlite3_finalize(st);
        return ROUND_DAO_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Round *tmp = realloc(rounds_array, sizeof(Round)* new_cap); 

            if(!tmp) {
                free(rounds_array);
                sqlite3_finalize(st);
                return ROUND_DAO_MALLOC_ERROR;
            }

            rounds_array = tmp; 
            cap = new_cap;
        }

        Round r;

        r.id_round = sqlite3_column_int64(st,0);
        r.id_game = sqlite3_column_int64(st, 1);
        const unsigned char *state = sqlite3_column_text(st, 2);
        r.duration = sqlite3_column_int64(st,3);
        const unsigned char *board = sqlite3_column_text(st,4);

        if (state) {
            r.state = string_to_round_status((const char*) state);
        } else {
            r.state = ROUND_STATUS_INVALID;
        }

        if(board)
            strcpy(r.board, (const char*) board);
        else {
            r.board[0] = '\0';
        }

        rounds_array[count++] = r;
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(rounds_array);
        sqlite3_finalize(st);
        return ROUND_DAO_SQL_ERROR;
    }

    *out_array = rounds_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return ROUND_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_DAO_SQL_ERROR;

}

RoundDaoStatus update_round_by_id(sqlite3 *db, const Round *upd_round) {

    if (db == NULL || upd_round == NULL || upd_round->id_round <= 0) {
        return ROUND_DAO_INVALID_INPUT;
    }

    Round original_round;
    RoundDaoStatus round_status = get_round_by_id(db, upd_round->id_round, &original_round);

    if (round_status != ROUND_DAO_OK) {
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

    if(strcmp(original_round.board, upd_round->board) != 0) {
        flags |= UPDATE_ROUND_BOARD;
    }

    if (flags == 0) {
        return ROUND_DAO_NOT_MODIFIED;
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

    if (flags & UPDATE_ROUND_BOARD) {
        if (!first) strcat(query, ", ");
        strcat(query, "board = ?");
        first = false;
    }

    strcat(query, " WHERE id_round = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    if (flags & UPDATE_ROUND_ID_GAME) {
        rc = sqlite3_bind_int64(st, param_index++, upd_round->id_game);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_ROUND_STATE) {
        rc = sqlite3_bind_text(st, param_index++, round_status_to_string(upd_round->state), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_ROUND_DURATION) {
        rc = sqlite3_bind_int64(st, param_index++, upd_round->duration);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_ROUND_BOARD) {
        rc = sqlite3_bind_text(st, param_index++, upd_round->board, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    rc = sqlite3_bind_int64(st, param_index, upd_round->id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return ROUND_DAO_NOT_MODIFIED;
    
    return ROUND_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return ROUND_DAO_SQL_ERROR;
}

RoundDaoStatus delete_round_by_id(sqlite3 *db, int64_t id_round) {

    if (db == NULL || id_round <= 0) {
        return ROUND_DAO_INVALID_INPUT;
    }

    const char *query = "DELETE FROM Round WHERE id_round = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int64(stmt, 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return ROUND_DAO_NOT_FOUND;

    return ROUND_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_DAO_SQL_ERROR;
}

RoundDaoStatus insert_round(sqlite3 *db, Round *in_out_round) {

    if (db == NULL || in_out_round == NULL) {
        return ROUND_DAO_INVALID_INPUT;
    }

    if (in_out_round->id_game <= 0 || in_out_round->duration < 0) {
        return ROUND_DAO_INVALID_INPUT;
    }

    if (in_out_round->state < ACTIVE_ROUND || in_out_round->state > FINISHED_ROUND) {
        return ROUND_DAO_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char *query = 
        "INSERT INTO Round (id_game, state, duration, board)"
        " VALUES ( ?1, ?2, ?3, ?4) RETURNING id_round";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    rc = sqlite3_bind_int64(stmt, param_index++, in_out_round->id_game);
    if (rc != SQLITE_OK) goto bind_fail;

    const char *r_st = round_status_to_string(in_out_round->state);
    if(!r_st) {
        sqlite3_finalize(stmt);
        return ROUND_DAO_INVALID_INPUT;
    }

    rc = sqlite3_bind_text(stmt, param_index++, r_st, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, param_index++, in_out_round->duration);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_text(stmt, param_index++, in_out_round->board, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_ROW) goto step_fail;

    in_out_round->id_round = sqlite3_column_int64(stmt, 0);;

    sqlite3_finalize(stmt);
    return ROUND_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return ROUND_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ROUND_DAO_SQL_ERROR;
}
