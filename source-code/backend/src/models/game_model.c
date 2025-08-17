#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game_model.h"


const char* game_status_to_string(GameStatus state) {
    switch (state) {
        case NEW :              return "new";
        case ACTIVE :           return "active";
        case WAITING :          return "waiting";
        case FINISHED :         return "finished";    
        default:                return "new";
    }
}

const char* return_game_status_to_string(GameReturnStatus status) {
    switch (status) {
        case GAME_OK:             return "GAME_OK";
        case GAME_INVALID_INPUT:  return "GAME_INVALID_INPUT";
        case GAME_SQL_ERROR:      return "GAME_SQL_ERROR";
        case GAME_NOT_FOUND:      return "GAME_NOT_FOUND";
        default:                  return "GAME_UNKNOWN";
    }
}

GameStatus string_to_game_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "new") == 0) return NEW;
        if (strcmp(state_str, "active") == 0) return ACTIVE;
        if (strcmp(state_str, "waiting") == 0) return WAITING;
        if (strcmp(state_str, "finished") == 0) return FINISHED;
    }

    return NEW; 
}

GameReturnStatus get_game_by_id(sqlite3 *db, int id_game, Game *out) {

    if(db == NULL || id_game <= 0 || out == NULL) {
        return GAME_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_game, id_creator, id_owner, state, created_at "
        "FROM Game WHERE id_game = ?1";

    sqlite3_stmt *st = NULL;    

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);

    if (rc != SQLITE_OK) return GAME_SQL_ERROR;

    rc = sqlite3_bind_int(st , 1, id_game);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(st);
        return GAME_SQL_ERROR;
    }

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_game = sqlite3_column_int(st, 0); 
        out->id_creator = sqlite3_column_int(st, 1); 
        out->id_owner = sqlite3_column_int(st, 2); 
        const unsigned char *state = sqlite3_column_text(st, 3); 
        const unsigned char *created_at = sqlite3_column_text(st, 4);


        if (state) {
            out->state = string_to_game_status((const char*) state);
        }

        if (created_at) {
            strcpy(out->created_at, (const char*) created_at);
        }

        sqlite3_finalize(st); 
        return GAME_OK;

    } else if (rc == SQLITE_DONE) { 

        sqlite3_finalize(st);
        return GAME_NOT_FOUND;

    } else {

        sqlite3_finalize(st);
        return GAME_SQL_ERROR;
    }    
}

GameReturnStatus get_all_games(sqlite3 *db, Game** out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { 
        return GAME_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_game, id_creator, id_owner, state, created_at FROM Game"; 

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);

    if (rc != SQLITE_OK) return GAME_SQL_ERROR;

    int cap = 16; 

    Game *games_array = malloc(sizeof(Game) * cap);

    if (!games_array) {
        sqlite3_finalize(st);
        return GAME_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Game *tmp = realloc(games_array, sizeof(Game)* new_cap); 

            if(!tmp) {
                free(games_array);
                sqlite3_finalize(st);
                return GAME_MALLOC_ERROR;
            }

            games_array = tmp; 
            cap = new_cap;
        }

        Game g;

        g.id_game = sqlite3_column_int(st,0);
        g.id_creator = sqlite3_column_int(st, 1);
        g.id_owner = sqlite3_column_int(st,2);
        g.state = string_to_game_status((const char*)sqlite3_column_text(st,3));
        strcpy(g.created_at, (const char*) sqlite3_column_text(st, 4));

        games_array[count++] = g;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(games_array);
        sqlite3_finalize(st);
        return GAME_SQL_ERROR;
    }

    *out_array = games_array; 
    *out_count = count;

    sqlite3_finalize(st);

    return GAME_OK;

}

GameReturnStatus update_game_by_id(sqlite3 *db, const Game *upd_game) {

    if (db == NULL || upd_game == NULL) {
        return GAME_INVALID_INPUT;
    }

    Game original_game;
    GameReturnStatus game_status = get_game_by_id(db, upd_game->id_game, &original_game);

    if (game_status != GAME_OK) {
        return game_status;
    }

    UpdateGameFlags flags = 0; 

    if(original_game.id_creator != upd_game->id_creator) {
        flags |= UPDATE_ID_CREATOR;
    }


    if(original_game.id_owner != upd_game->id_owner) {
        flags |= UPDATE_ID_OWNER;
    }

    if(original_game.state != upd_game->state) { //We can use != operator because enum type are integers 
        flags |= UPDATE_STATE;
    }

    if(strcmp(original_game.created_at, upd_game->created_at) != 0) {
        flags |= UPDATE_CREATED_AT;
    }

    if (flags == 0) {
        return GAME_NOT_MODIFIED;
    }

    char query[512] = "UPDATE Game SET ";

    sqlite3_stmt *st = NULL;

    bool first = true; 

    if (flags & UPDATE_ID_CREATOR) { 
        if (!first) strcat(query, ", "); //If it isn't the first it adds the "," and then adds the correct column
        strcat(query, "id_creator = ?"); //We won't have the "," at the end becuase it added earlier
        first = false;
    }

    if(flags & UPDATE_ID_OWNER) {
        if (!first) strcat(query, ", ");
        strcat(query, "id_owner = ?");
        first = false;
    }

    if (flags & UPDATE_STATE) {
        if (!first) strcat(query, ", ");
        strcat(query, "state = ?");
        first = false;
    }

    if (flags & UPDATE_CREATED_AT) {
        if (!first) strcat(query, ", ");
        strcat(query, "created_at = ?");
        first = false;
    }

    strcat(query, " WHERE id_game = ?");

    printf("\n\nQUERY\n\n: %s", query);

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); 

    if (rc != SQLITE_OK) {
        if (st) sqlite3_finalize(st);
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        return GAME_SQL_ERROR;
    }

    int param_index = 1;

    if (flags & UPDATE_ID_CREATOR) {
        sqlite3_bind_int(st, param_index++, upd_game->id_creator);
    }

    if (flags & UPDATE_ID_OWNER) {
        sqlite3_bind_int(st, param_index++, upd_game->id_owner);
    }

    if (flags & UPDATE_STATE) {
        sqlite3_bind_text(st, param_index++, game_status_to_string(upd_game->state), -1, SQLITE_STATIC);
    }

    if (flags & UPDATE_CREATED_AT) {
        sqlite3_bind_text(st, param_index++, upd_game->created_at, -1, SQLITE_STATIC);
    }

    sqlite3_bind_int(st, param_index, upd_game->id_game);


    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        return GAME_SQL_ERROR;
    }
    
    return GAME_OK;
}

GameReturnStatus delete_game_by_id(sqlite3 *db, int id_game) {

    if (db == NULL || id_game <= 0) {
        return GAME_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Game WHERE id_game = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return GAME_SQL_ERROR;
    }

    sqlite3_bind_int(stmt, 1, id_game);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        if (stmt) {
            fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return GAME_SQL_ERROR;
        }
    }

    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) return GAME_NOT_FOUND;

    return GAME_OK;
}

GameReturnStatus insert_game(sqlite3 *db, const Game *in_game) {

    if (db == NULL || in_game == NULL) {
        return GAME_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char* query = 
        "INSERT INTO Game (id_creator, id_owner, state, created_at)"
        " VALUES ( ?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        return GAME_SQL_ERROR;
    }

    int param_index = 1;

    if (in_game->id_creator <= 0) {
        sqlite3_finalize(stmt);
        return GAME_INVALID_INPUT;
    } 

    sqlite3_bind_int(stmt, param_index++, in_game->id_creator);

    if (in_game->id_owner <= 0) {
        sqlite3_finalize(stmt);
        return GAME_INVALID_INPUT;
    }

    sqlite3_bind_int(stmt, param_index++, in_game->id_owner);

    const char* g_st = game_status_to_string(in_game->state);

    if (!g_st || (strcmp(g_st, "new") != 0 && strcmp(g_st, "active")  != 0 && strcmp(g_st, "waiting") != 0 && strcmp(g_st, "finished")!= 0)) {
        sqlite3_finalize(stmt);
        return GAME_INVALID_INPUT;
    }

    sqlite3_bind_text(stmt, param_index++, game_status_to_string(in_game->state), -1, SQLITE_STATIC);

    if(in_game->created_at[0] == '\0') {
        sqlite3_finalize(stmt);
        return GAME_INVALID_INPUT;
    }

    sqlite3_bind_text(stmt, param_index++, in_game->created_at, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        return GAME_SQL_ERROR;
    }

    if (sqlite3_changes(db) == 0) return GAME_NOT_MODIFIED;

    sqlite3_finalize(stmt);
    return GAME_OK;
}




