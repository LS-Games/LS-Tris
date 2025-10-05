#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../../include/debug_log.h"

#include "play_dao_sqlite.h"

const char* return_play_dao_status_to_string(PlayDaoStatus status) {
    switch (status) {
        case PLAY_DAO_OK:               return "PLAY_DAO_OK";
        case PLAY_DAO_INVALID_INPUT:    return "PLAY_DAO_INVALID_INPUT";
        case PLAY_DAO_SQL_ERROR:        return "PLAY_DAO_SQL_ERROR";
        case PLAY_DAO_NOT_FOUND:        return "PLAY_DAO_NOT_FOUND";
        default:                        return "PLAY_DAO_UNKNOWN";
    }
}

PlayDaoStatus get_play_by_pk(sqlite3 *db, int64_t id_player, int64_t id_round, Play *out) {

    if(db == NULL || id_player <= 0 || id_round <= 0 || out == NULL) {
        return PLAY_DAO_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT result, player_number"
        " FROM Play WHERE id_player = ?1 AND id_round = ?2";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(st , 1, id_player);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(st , 2, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        const unsigned char *result = sqlite3_column_text(st, 0); 
        out->id_player = id_player;
        out -> id_round = id_round;
        out->player_number = sqlite3_column_int(st, 1);

        if (result) {
            out->result = string_to_play_result((const char*) result);
        } else {
            out->result = PLAY_RESULT_INVALID;
        }

        sqlite3_finalize(st); 
        return PLAY_DAO_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return PLAY_DAO_NOT_FOUND;

    } else goto step_fail;
    
    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus get_all_plays(sqlite3 *db, Play** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return PLAY_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_player, id_round, result, player_number FROM Play"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    Play *plays_array = malloc(sizeof(Play) * cap);

    if (!plays_array) {
        sqlite3_finalize(st);
        return PLAY_DAO_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Play *tmp = realloc(plays_array, sizeof(Play)* new_cap); 

            if(!tmp) {
                free(plays_array);
                sqlite3_finalize(st);
                return PLAY_DAO_MALLOC_ERROR;
            }

            plays_array = tmp; 
            cap = new_cap;
        }

        Play play;

        play.id_player = sqlite3_column_int64(st,0);
        play.id_round = sqlite3_column_int64(st, 1);
        const unsigned char *result = sqlite3_column_text(st, 2);
        play.player_number = sqlite3_column_int(st , 3);

        if (result) {
            play.result = string_to_play_result((const char*) result);
        } else {
            play.result = PLAY_RESULT_INVALID;
        }

        plays_array[count++] = play;
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(plays_array);
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
    }

    *out_array = plays_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus update_play_by_pk(sqlite3 *db, Play *upd_play) {

    if (db == NULL || upd_play == NULL || upd_play->id_player <= 0 || upd_play->id_round <= 0 || upd_play->result < WIN || upd_play->result > DRAW) {
        return PLAY_DAO_INVALID_INPUT;
    }

    Play original_play;
    PlayDaoStatus play_status = get_play_by_pk(db, upd_play->id_player, upd_play->id_round, &original_play);

    if (play_status != PLAY_DAO_OK) return play_status;

    const char *res_str = play_result_to_string(upd_play->result);
    const char *original_res_str = play_result_to_string(original_play.result);

    if (!res_str || !original_res_str) return PLAY_DAO_INVALID_INPUT;

    int flag = 0;
    int param_index = 1;
    bool first = true;
    char query[512] = "UPDATE Play SET ";

    if (strcmp(res_str, original_res_str) != 0) {
        flag |= UPDATE_PLAY_RESULT;
    }

    if ( original_play.player_number != upd_play->player_number) {
        flag |= UPDATE_PLAY_PLAYER_NUMBER;
    }

    if (flag == 0) return PLAY_DAO_NOT_MODIFIED;

    if (flag & UPDATE_PLAY_RESULT) {
        if (!first) strcat(query, ", ");
        strcat(query, "result = ?");
        first = false;
    }

    if (flag & UPDATE_PLAY_PLAYER_NUMBER) {
        if (!first) strcat(query, ", ");
        strcat(query, "player_number = ?");
        first = false;
    }

    strcat(query, " WHERE id_player = ? AND id_round = ? RETURNING id_player, id_round, result, player_number");

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    if (flag & UPDATE_PLAY_RESULT) {
        rc = sqlite3_bind_text(st, param_index++, res_str, -1, SQLITE_TRANSIENT);
        if ( rc != SQLITE_OK) goto bind_fail;
    }

    if (flag & UPDATE_PLAY_PLAYER_NUMBER) {
        rc = sqlite3_bind_int(st, param_index++, upd_play->player_number);
        if ( rc != SQLITE_OK) goto bind_fail;
    }

    rc = sqlite3_bind_int64(st, param_index++, upd_play->id_player);
    if ( rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(st, param_index++, upd_play->id_round);
    if ( rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);
    if (rc != SQLITE_ROW) goto step_fail; 

    upd_play->id_player = (int64_t) sqlite3_column_int64(st, 0);
    upd_play->id_round = (int64_t) sqlite3_column_int64(st, 1);
    unsigned const char* result = sqlite3_column_text(st, 2);
    upd_play->player_number = sqlite3_column_int(st,3);

    upd_play->result = string_to_play_result((const char*) result);

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return PLAY_DAO_NOT_MODIFIED;
    
    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus delete_play_by_pk(sqlite3 *db, int64_t id_player, int64_t id_round) {

    if (db == NULL || id_player <= 0 || id_round <= 0) {
        return PLAY_DAO_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Play WHERE id_player = ?1 AND id_round = ?2";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int64(stmt, 1, id_player);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_bind_int64(stmt, 2, id_round);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return PLAY_DAO_NOT_FOUND;

    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus insert_play(sqlite3 *db, Play *in_out_play) {

    if (db == NULL || in_out_play == NULL) {
        return PLAY_DAO_INVALID_INPUT;
    }

    if (in_out_play->id_player <= 0 || in_out_play->id_round <= 0 || in_out_play->result < WIN || in_out_play->result > DRAW) {
        return PLAY_DAO_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char* query = 
        "INSERT INTO Play (id_player, id_round, result, player_number)"
        " VALUES ( ?1, ?2, ?3, ?4) RETURNING id_player, id_round, result, player_number";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(stmt, 1, in_out_play->id_player);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, 2, in_out_play->id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    const char* p_st = play_result_to_string(in_out_play->result);
    if(!p_st) {
        sqlite3_finalize(stmt);
        return PLAY_DAO_INVALID_INPUT;
    }

    rc = sqlite3_bind_text(stmt, 3, p_st, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(stmt, 4, in_out_play->player_number);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_ROW) goto step_fail;

    in_out_play->id_player = sqlite3_column_int64(stmt, 0);
    in_out_play->id_round = sqlite3_column_int64(stmt, 1);
    unsigned const char* result = sqlite3_column_text(stmt,2);
    in_out_play->result = string_to_play_result((const char*) result);
    in_out_play->player_number = sqlite3_column_int(stmt, 3);

    sqlite3_finalize(stmt);
    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus get_all_plays_by_round(sqlite3 *db, Play** out_array, int64_t id_round, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL || id_round <= 0) { 
        return PLAY_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_player, id_round, result, player_number "
                      "FROM Play "
                      "WHERE y.id_round = ?1 "
                      "ORDER BY y.id_round ASC;"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(st, 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    int cap = 2; 

    Play *plays_array = malloc(sizeof(Play) * cap);

    if (!plays_array) {
        sqlite3_finalize(st);
        return PLAY_DAO_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Play *tmp = realloc(plays_array, sizeof(Play)* new_cap); 

            if(!tmp) {
                free(plays_array);
                sqlite3_finalize(st);
                return PLAY_DAO_MALLOC_ERROR;
            }

            plays_array = tmp; 
            cap = new_cap;
        }

        Play play;

        play.id_player = sqlite3_column_int64(st,0);
        play.id_round = sqlite3_column_int64(st, 1);
        const unsigned char *result = sqlite3_column_text(st, 2);
        play.player_number = sqlite3_column_int(st , 3);

        if (result) {
            play.result = string_to_play_result((const char*) result);
        } else {
            play.result = PLAY_RESULT_INVALID;
        }

        plays_array[count++] = play;
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(plays_array);
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
    }

    *out_array = plays_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
}

PlayDaoStatus get_all_plays_with_player_info(sqlite3 *db, PlayWithPlayerNickname** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return PLAY_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT y.id_player, y.id_round, y.result, y.player_number, p.nickname AS player_nickname "
                      "FROM Play y JOIN Player p ON y.id_player = p.id_player "
                      "ORDER BY y.id_round ASC;"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    PlayWithPlayerNickname *plays_array = malloc(sizeof(PlayWithPlayerNickname) * cap);

    if (!plays_array) {
        sqlite3_finalize(st);
        return PLAY_DAO_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            PlayWithPlayerNickname *tmp = realloc(plays_array, sizeof(PlayWithPlayerNickname)* new_cap); 

            if(!tmp) {
                free(plays_array);
                sqlite3_finalize(st);
                return PLAY_DAO_MALLOC_ERROR;
            }

            plays_array = tmp; 
            cap = new_cap;
        }

        PlayWithPlayerNickname play_with_player_nickname;

        play_with_player_nickname.id_player = sqlite3_column_int64(st,0);
        play_with_player_nickname.id_round = sqlite3_column_int64(st, 1);
        unsigned const char *result = sqlite3_column_text(st, 2);
        play_with_player_nickname.player_number = sqlite3_column_int(st , 3);
        unsigned const char *player_nickname = sqlite3_column_text(st, 4);
        snprintf(play_with_player_nickname.player_nickname, sizeof play_with_player_nickname.player_nickname, "%s", player_nickname ? (const char*) player_nickname : "");

        if (result) {
            play_with_player_nickname.result = string_to_play_result((const char*) result);
        } else {
            play_with_player_nickname.result = PLAY_RESULT_INVALID;
        }

        plays_array[count++] = play_with_player_nickname;
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(plays_array);
        sqlite3_finalize(st);
        return PLAY_DAO_SQL_ERROR;
    }

    *out_array = plays_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAY_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAY_DAO_SQL_ERROR;
}