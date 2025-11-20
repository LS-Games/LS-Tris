#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../../include/debug_log.h"

#include "game_dao_sqlite.h"

const char *return_game_dao_status_to_string(GameDaoStatus status) {
    switch (status) {
        case GAME_DAO_OK:               return "GAME_DAO_OK";
        case GAME_DAO_INVALID_INPUT:    return "GAME_DAO_INVALID_INPUT";
        case GAME_DAO_SQL_ERROR:        return "GAME_DAO_SQL_ERROR";
        case GAME_DAO_NOT_FOUND:        return "GAME_DAO_NOT_FOUND";
        default:                        return "GAME_DAO_UNKNOWN";
    }
}

GameDaoStatus get_game_by_id(sqlite3 *db, int64_t id_game, Game *out) {

    if(db == NULL || id_game <= 0 || out == NULL) {
        return GAME_DAO_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_game, id_creator, id_owner, state, unixepoch(created_at) "
        "FROM Game WHERE id_game = ?1";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(st , 1, id_game);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_game = sqlite3_column_int64(st, 0); 
        out->id_creator = sqlite3_column_int64(st, 1); 
        out->id_owner = sqlite3_column_int64(st, 2); 
        const unsigned char *state = sqlite3_column_text(st, 3); 
        out->created_at = (time_t) sqlite3_column_int64(st, 4);

        if (state) {
            out->state = string_to_game_status((const char*) state);
        } else {
            out->state = GAME_STATUS_INVALID;
        }

        sqlite3_finalize(st); 
        return GAME_DAO_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return GAME_DAO_NOT_FOUND;

    } else goto step_fail;
    
    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return GAME_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return GAME_DAO_SQL_ERROR;
}

GameDaoStatus get_all_games(sqlite3 *db, Game **out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return GAME_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_game, id_creator, id_owner, state, unixepoch(created_at) FROM Game"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; 

    Game *games_array = malloc(sizeof(Game) * cap);

    if (!games_array) {
        sqlite3_finalize(st);
        return GAME_DAO_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Game *tmp = realloc(games_array, sizeof(Game)* new_cap); 

            if(!tmp) {
                free(games_array);
                sqlite3_finalize(st);
                return GAME_DAO_MALLOC_ERROR;
            }

            games_array = tmp; 
            cap = new_cap;
        }

        Game g;

        g.id_game = sqlite3_column_int64(st,0);
        g.id_creator = sqlite3_column_int64(st, 1);
        g.id_owner = sqlite3_column_int64(st,2);
        const unsigned char *state = sqlite3_column_text(st, 3);
        g.created_at = (time_t) sqlite3_column_int64(st,4);

        if (state) {
            g.state = string_to_game_status((const char*) state);
        } else {
            g.state = GAME_STATUS_INVALID;
        }

        games_array[count++] = g;
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("DATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(games_array);
        sqlite3_finalize(st);
        return GAME_DAO_SQL_ERROR;
    }

    *out_array = games_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return GAME_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;

}

GameDaoStatus update_game_by_id(sqlite3 *db, const Game *upd_game) {

    if (db == NULL || upd_game == NULL || upd_game->id_game <= 0) {
        return GAME_DAO_INVALID_INPUT;
    }

    Game original_game;
    GameDaoStatus game_status = get_game_by_id(db, upd_game->id_game, &original_game);

    if (game_status != GAME_DAO_OK) {
        return game_status;
    }

    UpdateGameFlags flags = 0; 

    if(original_game.id_creator != upd_game->id_creator) {
        flags |= UPDATE_GAME_ID_CREATOR;
    }


    if(original_game.id_owner != upd_game->id_owner) {
        flags |= UPDATE_GAME_ID_OWNER;
    }

    if(original_game.state != upd_game->state) { //We can use != operator because enum type are integers 
        flags |= UPDATE_GAME_STATE;
    }

    if(original_game.created_at != upd_game->created_at) {
        flags |= UPDATE_GAME_CREATED_AT;
    }

    if (flags == 0) {
        return GAME_DAO_NOT_MODIFIED;
    }

    char query[512] = "UPDATE Game SET ";

    sqlite3_stmt *st = NULL;

    bool first = true; 

    if (flags & UPDATE_GAME_ID_CREATOR) { 
        if (!first) strcat(query, ", "); 
        strcat(query, "id_creator = ?"); 
        first = false;
    }

    if(flags & UPDATE_GAME_ID_OWNER) {
        if (!first) strcat(query, ", ");
        strcat(query, "id_owner = ?");
        first = false;
    }

    if (flags & UPDATE_GAME_STATE) {
        if (!first) strcat(query, ", ");
        strcat(query, "state = ?");
        first = false;
    }

    if (flags & UPDATE_GAME_CREATED_AT) {
        if (!first) strcat(query, ", ");
        strcat(query, "created_at = datetime(?, 'unixepoch')");
        first = false;
    }

    strcat(query, " WHERE id_game = ? ");

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    if (flags & UPDATE_GAME_ID_CREATOR) {
        rc = sqlite3_bind_int64(st, param_index++, upd_game->id_creator);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_GAME_ID_OWNER) {
        rc = sqlite3_bind_int64(st, param_index++, upd_game->id_owner);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_GAME_STATE) {
        rc = sqlite3_bind_text(st, param_index++, game_status_to_string(upd_game->state), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_GAME_CREATED_AT) {
        rc = sqlite3_bind_int64(st, param_index++, (sqlite3_int64) upd_game->created_at);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    rc = sqlite3_bind_int64(st, param_index, upd_game->id_game);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return GAME_DAO_NOT_MODIFIED;
    
    return GAME_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return GAME_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return GAME_DAO_SQL_ERROR;
}

GameDaoStatus delete_game_by_id(sqlite3 *db, int64_t id_game) {

    if (db == NULL || id_game <= 0) {
        return GAME_DAO_INVALID_INPUT;
    }

    const char *query = "DELETE FROM Game WHERE id_game = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int64(stmt, 1, id_game);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return GAME_DAO_NOT_FOUND;

    return GAME_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;
    
    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;
}

GameDaoStatus insert_game(sqlite3 *db, Game *in_out_game) {

    if (db == NULL || in_out_game == NULL) {
        return GAME_DAO_INVALID_INPUT;
    }

    if (in_out_game->id_creator <= 0 || in_out_game->id_owner <= 0 || in_out_game->created_at == 0) {
        return GAME_DAO_INVALID_INPUT;
    }

    if (in_out_game->state < NEW_GAME || in_out_game->state > FINISHED_GAME) {
        return GAME_DAO_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char *query = 
        "INSERT INTO Game (id_creator, id_owner, state, created_at)"
        " VALUES ( ?, ?, ?, datetime(?,'unixepoch')) RETURNING id_game, id_creator, id_owner, state, unixepoch(created_at)";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    rc = sqlite3_bind_int64(stmt, param_index++, in_out_game->id_creator);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, param_index++, in_out_game->id_owner);
    if (rc != SQLITE_OK) goto bind_fail;

    const char *g_st = game_status_to_string(in_out_game->state);
    if(!g_st) {
        sqlite3_finalize(stmt);
        return GAME_DAO_INVALID_INPUT;
    }

    rc = sqlite3_bind_text(stmt, param_index++, g_st, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, param_index++, (sqlite3_int64) in_out_game->created_at);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_ROW) goto step_fail;

    in_out_game->id_game = sqlite3_column_int64(stmt, 0);
    in_out_game->id_creator = sqlite3_column_int64(stmt, 1);
    in_out_game->id_owner = sqlite3_column_int64(stmt, 2);
    const unsigned char *state = sqlite3_column_text(stmt, 3);
    in_out_game->state = string_to_game_status((const char*) state);
    in_out_game->created_at = (time_t) sqlite3_column_int64(stmt,4);

    sqlite3_finalize(stmt);
    return GAME_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;

    bind_fail:
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;

    step_fail:
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;
}

GameDaoStatus get_game_by_id_with_player_info(sqlite3 *db, int64_t id_game, GameWithPlayerNickname *out) {

    if (db == NULL || out == NULL) {
        return GAME_DAO_INVALID_INPUT;
    }

    const char *query =
        "SELECT "
        " o.nickname              AS owner, "
        " c.nickname              AS creator, "
        " g.id_game               AS id_game, "
        " g.id_creator            AS id_creator, "
        " g.state                 AS state, "
        " unixepoch(g.created_at) AS created_at "
        "FROM Game g "
        "JOIN Player c ON c.id_player = g.id_creator "
        "JOIN Player o ON o.id_player = g.id_owner "
        "WHERE g.id_game = ?;";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;
    }

    rc = sqlite3_bind_int64(stmt, 1, id_game);
    if (rc != SQLITE_OK) {
        LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {

        const unsigned char *owner      = sqlite3_column_text(stmt, 0);
        const unsigned char *creator    = sqlite3_column_text(stmt, 1);

        out->id_game    = sqlite3_column_int64(stmt, 2);
        out->id_creator = sqlite3_column_int64(stmt, 3);

        const unsigned char *stateText  = sqlite3_column_text(stmt, 4);
        out->created_at = (time_t) sqlite3_column_int64(stmt, 5);

        snprintf(out->owner,   sizeof(out->owner),   "%s", owner   ? (const char*)owner   : "");
        snprintf(out->creator, sizeof(out->creator), "%s", creator ? (const char*)creator : "");

        out->state = string_to_game_status((const char*)stateText);

        sqlite3_finalize(stmt);
        return GAME_DAO_OK;

    } else if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return GAME_DAO_NOT_FOUND;
    } else {
        LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;
    }
}


GameDaoStatus get_all_games_with_player_info(sqlite3 *db, GameWithPlayerNickname **out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) {
        return GAME_DAO_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0;
    int cap = 12;

    const char *query = "SELECT "
                        " o.nickname                 AS owner, "
                        " c.nickname                 AS creator, " 
                        " g.id_game                  AS id_game, "
                        " g.state                    AS state, "
                        " unixepoch(g.created_at)    AS created_at " 
                      "FROM Game g "
                      "JOIN Player c ON c.id_player = g.id_creator "
                      "JOIN Player o ON o.id_player = g.id_owner "
                      "ORDER BY g.created_at DESC, g.id_game DESC;";

    sqlite3_stmt* stmt = NULL;
    
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    GameWithPlayerNickname *array = malloc(sizeof(GameWithPlayerNickname) * cap);
    int count = 0;

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

            if (count == cap) {
                int new_cap = cap * 2;
                GameWithPlayerNickname *tmp = realloc(array, sizeof(GameWithPlayerNickname) * new_cap);

                if(!tmp) {
                    free(array);
                    sqlite3_finalize(stmt);
                    return GAME_DAO_MALLOC_ERROR;
                }

                array = tmp;
                cap = new_cap;
            }

            int col = 0; 

            unsigned const char *owner = sqlite3_column_text(stmt, col++);
            snprintf(array[count].owner, sizeof array[count].owner, "%s", owner ? (const char*) owner : "");

            unsigned const char *creator = sqlite3_column_text(stmt, col++);
            snprintf(array[count].creator, sizeof array[count].creator, "%s", creator ? (const char*) creator : "");

            array[count].id_game = sqlite3_column_int64(stmt, col++);

            unsigned const char *state = sqlite3_column_text(stmt, col++);
            array[count].state = string_to_game_status((const char*) state);

            array[count].created_at = (time_t) sqlite3_column_int64(stmt, col++);

            count++;
    }

    if(rc != SQLITE_DONE) {
        LOG_ERROR("DATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(array);
        sqlite3_finalize(stmt);
        return GAME_DAO_SQL_ERROR;
    }

    sqlite3_finalize(stmt);
    *out_array = array;
    *out_count = count;
    return GAME_DAO_OK;

    prepare_fail:
        LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return GAME_DAO_SQL_ERROR;
}
