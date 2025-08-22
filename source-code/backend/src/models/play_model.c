#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "play_model.h"

const char* play_result_to_string(PlayResult result) {
    switch (result) {
        case WIN :              return "win";
        case LOSE :             return "lose";
        case DRAW :             return "draw";
        default:                return NULL;
    }
}

const char* return_play_status_to_string(PlayReturnStatus status) {
    switch (status) {
        case PLAY_OK:             return "PLAY_OK";
        case PLAY_INVALID_INPUT:  return "PLAY_INVALID_INPUT";
        case PLAY_SQL_ERROR:      return "PLAY_SQL_ERROR";
        case PLAY_NOT_FOUND:      return "PLAY_NOT_FOUND";
        default:                  return "PLAY_UNKNOWN";
    }
}

PlayResult string_to_play_result(const char *result_str) {
    if (result_str) {
        if (strcmp(result_str, "win") == 0) return WIN;
        if (strcmp(result_str, "lose") == 0) return LOSE;
        if (strcmp(result_str, "draw") == 0) return DRAW;
    }

    return PLAY_RESULT_INVALID; 
}

PlayReturnStatus get_play_by_pk(sqlite3 *db, int id_player, int id_round, Play *out) {

    if(db == NULL || id_player <= 0 || id_round <= 0 || out == NULL) {
        return PLAY_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT result"
        " FROM Play WHERE id_player = ?1 AND id_round = ?2";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int(st , 1, id_player);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(st , 2, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        const unsigned char *result = sqlite3_column_text(st, 0); 
        out->id_player = id_player;
        out -> id_round = id_round;

        if (result) {
            out->result = string_to_play_result((const char*) result);
        } else {
            out->result = PLAY_RESULT_INVALID;
        }

        sqlite3_finalize(st); 
        return PLAY_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return PLAY_NOT_FOUND;

    } else goto step_fail;
    
    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_SQL_ERROR;
}

PlayReturnStatus get_all_plays(sqlite3 *db, Play** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return PLAY_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_player, id_round, result FROM Play"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    Play *plays_array = malloc(sizeof(Play) * cap);

    if (!plays_array) {
        sqlite3_finalize(st);
        return PLAY_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Play *tmp = realloc(plays_array, sizeof(Play)* new_cap); 

            if(!tmp) {
                free(plays_array);
                sqlite3_finalize(st);
                return PLAY_MALLOC_ERROR;
            }

            plays_array = tmp; 
            cap = new_cap;
        }

        Play play;

        play.id_player = sqlite3_column_int(st,0);
        play.id_round = sqlite3_column_int(st, 1);
        const unsigned char *result = sqlite3_column_text(st, 2);

        if (result) {
            play.result = string_to_play_result((const char*) result);
        } else {
            play.result = PLAY_RESULT_INVALID;
        }

        plays_array[count++] = play;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(plays_array);
        sqlite3_finalize(st);
        return PLAY_SQL_ERROR;
    }

    *out_array = plays_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAY_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_SQL_ERROR;

}

PlayReturnStatus update_play_by_pk(sqlite3 *db, const Play *upd_play) {

    if (db == NULL || upd_play == NULL || upd_play->id_player <= 0 || upd_play->id_round <= 0 || upd_play->result < WIN || upd_play->result > DRAW) {
        return PLAY_INVALID_INPUT;
    }

    Play original_play;
    PlayReturnStatus play_status = get_play_by_pk(db, upd_play->id_player, upd_play->id_round, &original_play);

    if (play_status != PLAY_OK) return play_status;

    if (original_play.result == upd_play->result) return PLAY_NOT_MODIFIED;

    const char *res_str = play_result_to_string(upd_play->result);
    if (!res_str) return PLAY_INVALID_INPUT;

    const char *query = "UPDATE Play SET result = ?1 WHERE id_player = ?2 AND id_round = ?3";

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_text(st, 1, res_str, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_bind_int(st, 2, upd_play->id_player);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_bind_int(st, 3, upd_play->id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return PLAY_NOT_MODIFIED;
    
    return PLAY_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_SQL_ERROR;
}

PlayReturnStatus delete_play_by_pk(sqlite3 *db, int id_player, int id_round) {

    if (db == NULL || id_player <= 0 || id_round <= 0) {
        return PLAY_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Play WHERE id_player = ?1 AND id_round = ?2";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int(stmt, 1, id_player);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_bind_int(stmt, 2, id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return PLAY_NOT_FOUND;

    return PLAY_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_SQL_ERROR;
}

PlayReturnStatus insert_play(sqlite3 *db, const Play *in_play) {

    if (db == NULL || in_play == NULL) {
        return PLAY_INVALID_INPUT;
    }

    if (in_play->id_player <= 0 || in_play->id_round <= 0 || in_play->result < WIN || in_play->result > DRAW) {
        return PLAY_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char* query = 
        "INSERT INTO Play (id_player, id_round, result)"
        " VALUES ( ?1, ?2, ?3)";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int(stmt, 1, in_play->id_player);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(stmt, 2, in_play->id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    const char* p_st = play_result_to_string(in_play->result);
    if(!p_st) {
        sqlite3_finalize(stmt);
        return PLAY_INVALID_INPUT;
    }

    rc = sqlite3_bind_text(stmt, 3, p_st, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_DONE) goto step_fail;

    if (sqlite3_changes(db) == 0) return PLAY_NOT_MODIFIED;

    sqlite3_finalize(stmt);
    return PLAY_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_SQL_ERROR;
}






