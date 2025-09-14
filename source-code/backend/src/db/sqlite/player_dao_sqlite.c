#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "player_dao_sqlite.h"

const char* return_player_status_to_string(PlayerReturnStatus status) {
    switch (status) {
        case PLAYER_OK:             return "PLAYER_OK";
        case PLAYER_INVALID_INPUT:  return "PLAYER_INVALID_INPUT";
        case PLAYER_SQL_ERROR:      return "PLAYER_SQL_ERROR";
        case PLAYER_NOT_FOUND:      return "PLAYER_NOT_FOUND";
        default:                    return "PLAYER_UNKNOWN";
    }
}

PlayerReturnStatus get_player_by_id(sqlite3 *db, int64_t id, Player *out) {

    if(db == NULL || id <= 0 || out == NULL) {
        return PLAYER_INVALID_INPUT;
    }

    //We have unixepoch(registration_date), which is a sqlite3 function that converts a TEXT db type to time_t entity value

    const char *sql = 
        "SELECT id_player, nickname, email, password, current_streak, max_streak, unixepoch(registration_date) "
        "FROM Player WHERE id_player = ?1";

    sqlite3_stmt *st = NULL;    //pointer to compiled query (statement)

    //This SQLite function requires:
    //  1 - database 
    //  2 - query SQL (that we saved in sql)
    //  3 - query length (with -1 it calculates it in automatically)
    //  4 - in this pointer the function saved the address of compiled object, the latter is obtained from the translation of the SQL string into bytecode for the SQLite engine.
    //  5 - used to find out where the query part used has ended up (is optional)

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    //This SQLite function used to replace placeholder in the query
    //In our case it replaces placeholder "1" with id value

    rc = sqlite3_bind_int64(st , 1, id);
    if (rc != SQLITE_OK) goto bind_fail;

    //This SQLite function executes the statement
    //In our case, we are using SELECT so the function should return a SQLITE_ROW if it finds a row

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_player = sqlite3_column_int64(st, 0); //It takes the value from column with index 0 and assigns it to id_player
        const unsigned char *nickname = sqlite3_column_text(st, 1); //The function returns a const unsigned char
        const unsigned char *email = sqlite3_column_text(st, 2);
        const unsigned char *password = sqlite3_column_text(st, 3);
        out->current_streak = sqlite3_column_int(st, 4);
        out->max_streak = sqlite3_column_int(st, 5);
        out->registration_date = (time_t) sqlite3_column_int64(st, 6);

        //Now we must save all the value returned by sqlite3_column_text function because they will be deleted when the statement closes
        //We cannot use the simple assignment operator (=) because the returned value is a pointer whereas the object fields are arrays of characters
        //We can use the strcpy() function to do it

        if (nickname) {
            strcpy(out->nickname, (const char*) nickname); //We do the cast because strcpy expects a const  
        } else {
            out->nickname[0] = '\0'; //So we avoid error when the returned value is NULL, ('\0' = NULL)
        }

        if (email) {
            strcpy(out->email, (const char*) email);
        } else {
            out->email[0] = '\0';
        }

        if (password) {
            strcpy(out->password, (const char*) password);
        } else {
            out->password[0] = '\0';
        }

        sqlite3_finalize(st); //We're closing the statement here
        return PLAYER_OK;

    } else if (rc == SQLITE_DONE) { //If slite3 return SQLITE_DONE and not SQLITE_ROW for SELECT operation, it means that the player was not found

        sqlite3_finalize(st);
        return PLAYER_NOT_FOUND;

    } else goto step_fail;  
    
    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAYER_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
}

PlayerReturnStatus get_all_players(sqlite3 *db, Player **out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { //We only check if pointers are null
        return PLAYER_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_player, nickname, email, password, current_streak, max_streak, unixepoch(registration_date) FROM Player"; //Avoid using SELECT *, so if the table will change we won't have problems with columns

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16; //arbitrary field

    Player *player_array = malloc(sizeof(Player) * cap);

    if (!player_array) {
        sqlite3_finalize(st);
        return PLAYER_MALLOC_ERROR;
    }

    int count = 0;

    while((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            int new_cap = cap * 2;
            Player *tmp = realloc(player_array, sizeof(Player)* new_cap); //We use realloc because we may alreay have some rows saved

            if(!tmp) {
                free(player_array);
                sqlite3_finalize(st);
                return PLAYER_MALLOC_ERROR;
            }

            player_array = tmp; 
            cap = new_cap;
        }

        Player p;

        p.id_player = sqlite3_column_int64(st,0);
        const unsigned char *nickname = sqlite3_column_text(st, 1);
        const unsigned char *email = sqlite3_column_text(st, 2);
        const unsigned char *password = sqlite3_column_text(st, 3);
        p.current_streak = sqlite3_column_int(st,4);
        p.max_streak = sqlite3_column_int(st,5);
        p.registration_date = (time_t) sqlite3_column_int64(st,6);

        if(nickname) {
            strcpy(p.nickname, (const char*) nickname);
        } else {
            p.nickname[0] = '\0';
        }

        if(email) {
            strcpy(p.email, (const char*) email);
        } else {
            p.email[0] = '\0';
        }

        if(password) {
            strcpy(p.password, (const char*) password);
        } else {
            p.password[0] = '\0';
        }

        player_array[count++] = p;
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "\nDATABASE ERROR: %s\n", sqlite3_errmsg(db));
        free(player_array);
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
    }

    *out_array = player_array; //We're assigning to the caller pointer the address of array 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAYER_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAYER_SQL_ERROR;
}

PlayerReturnStatus update_player_by_id(sqlite3 *db, const Player *upd_player) {

    if (db == NULL || upd_player == NULL) {
        return PLAYER_INVALID_INPUT;
    }

    //We retrieve the player saved in DB and compare it with the new one

    Player original_player;
    PlayerReturnStatus player_status = get_player_by_id(db, upd_player->id_player, &original_player);

    if (player_status != PLAYER_OK) {
        return player_status;
    }

    //We're comparing two objects and the flag are actived if they are different from each other

    UpdatePlayerFlags flags = 0; //00000000

    if (strcmp(original_player.nickname, upd_player->nickname) !=0) {
        flags |= UPDATE_PLAYER_NICKNAME; // 00000000 | 00000001 = 00000001
    }

    if (strcmp(original_player.email, upd_player->email) != 0) {
        flags |= UPDATE_PLAYER_EMAIL; // 00000000 | 00000010 = 00000010
    }

    if (strcmp(original_player.password, upd_player->password) != 0) {
        flags |= UPDATE_PLAYER_PASSWORD;
    }

    if (original_player.current_streak != upd_player->current_streak) {
        flags |= UPDATE_PLAYER_CURRENT_STREAK;
    }

    if (original_player.max_streak != upd_player->max_streak) {
        flags |= UPDATE_PLAYER_MAX_STREAK;
    }

    if (original_player.registration_date != upd_player->registration_date) {
        flags |= UPDATE_PLAYER_REG_DATE;
    }

    if (flags == 0) {
        return PLAYER_NOT_MODIFIED;
    }

    //Now we need to create the query dinamically 
    char query[512] = "UPDATE Player SET ";

    sqlite3_stmt *st = NULL;

    bool first = true; 

    //This condition checks if that nickname has been changed 

    //For example if we have a flag 00000101 it means that nickname and password have benn changed because (1 << 0 = 00000001) and (1 << 2 = 00000100)
    //So in the check we will have (00000101 AND 00000001 = 00000001) operation, the condition will be true

    if (flags & UPDATE_PLAYER_NICKNAME) { 
        if (!first) strcat(query, ", "); //If it isn't the first it adds the "," and then adds the correct column
        strcat(query, "nickname = ?"); //We won't have the "," at the end becuase it added earlier
        first = false;
    }

    if(flags & UPDATE_PLAYER_EMAIL) {
        if (!first) strcat(query, ", ");
        strcat(query, "email = ?");
        first = false;
    }

    if (flags & UPDATE_PLAYER_PASSWORD) {
        if (!first) strcat(query, ", ");
        strcat(query, "password = ?");
        first = false;
    }

    if (flags & UPDATE_PLAYER_CURRENT_STREAK) {
        if (!first) strcat(query, ", ");
        strcat(query, "current_streak = ?");
        first = false;
    }

    if (flags & UPDATE_PLAYER_MAX_STREAK) {
        if (!first) strcat(query, ", ");
        strcat(query, "max_streak = ?");
        first = false;
    }

    if (flags & UPDATE_PLAYER_REG_DATE) {
        if (!first) strcat(query, ", ");
        strcat(query, "registration_date = datetime(?, 'unixepoch')");
        first = false;
    }

    strcat(query, " WHERE id_player = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &st, NULL); //We have to do prepare before the bind
    if (rc != SQLITE_OK) goto prepare_fail;

    //After we builded the query we can prepare the statement
    int param_index = 1;

    if (flags & UPDATE_PLAYER_NICKNAME) {
        rc = sqlite3_bind_text(st, param_index++, upd_player->nickname, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_PLAYER_EMAIL) {
        rc = sqlite3_bind_text(st, param_index++, upd_player->email, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_PLAYER_PASSWORD) {
        rc =sqlite3_bind_text(st, param_index++, upd_player->password, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_PLAYER_CURRENT_STREAK) {
        rc =sqlite3_bind_int(st, param_index++, upd_player->current_streak);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_PLAYER_MAX_STREAK) {
        rc =sqlite3_bind_int(st, param_index++, upd_player->max_streak);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    if (flags & UPDATE_PLAYER_REG_DATE) {
        rc =sqlite3_bind_int64(st, param_index++, (sqlite3_int64) upd_player->registration_date);
        if (rc != SQLITE_OK) goto bind_fail; 
    }

    rc = sqlite3_bind_int64(st, param_index, upd_player->id_player);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) goto step_fail; 

    sqlite3_finalize(st);

    if (sqlite3_changes(db) == 0) return PLAYER_NOT_MODIFIED;
    
    return PLAYER_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAYER_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
} 

PlayerReturnStatus delete_player_by_id(sqlite3 *db, int64_t id) {

    if (db == NULL || id <= 0) {
        return PLAYER_INVALID_INPUT;
    }

    const char* query = "DELETE FROM Player WHERE id_player = ?1";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto prepare_fail; 

    rc = sqlite3_bind_int64(stmt, 1, id);
    if (rc != SQLITE_OK) goto bind_fail; 

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) goto step_fail;

    sqlite3_finalize(stmt);

    //If there have been no changes, then it has not been found
    if (sqlite3_changes(db) == 0) return PLAYER_NOT_FOUND;

    return PLAYER_OK;

    prepare_fail:
        fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
        return PLAYER_SQL_ERROR;
    
    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAYER_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAYER_SQL_ERROR;
}

PlayerReturnStatus insert_player(sqlite3* db, Player *in_out_player) {

    if (db == NULL || in_out_player == NULL) {
        return PLAYER_INVALID_INPUT;
    }

    sqlite3_stmt *stmt = NULL;

    const char* query = 
        "INSERT INTO Player (nickname, email, password, current_streak, max_streak, registration_date)"
        " VALUES ( ?, ?, ?, ?, ?, datetime(?,'unixepoch')) RETURNING id_player, nickname, email, password, current_streak, max_streak, unixepoch(registration_date)";

    if (in_out_player->nickname[0] == '\0' || in_out_player->email[0]    == '\0' ||
        in_out_player->password[0] == '\0' || in_out_player->current_streak < 0   ||
        in_out_player->max_streak    < 0   || in_out_player->registration_date == 0) {
        return PLAYER_INVALID_INPUT;
    }

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) goto prepare_fail;

    int param_index = 1;

    rc = sqlite3_bind_text(stmt, param_index++, in_out_player->nickname, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_text(stmt, param_index++, in_out_player->email, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_text(stmt, param_index++, in_out_player->password, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(stmt, param_index++, in_out_player->current_streak);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int(stmt, param_index++, in_out_player->max_streak);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_bind_int64(stmt, param_index++, (sqlite3_int64) in_out_player->registration_date);
    if (rc != SQLITE_OK) goto bind_fail;

    if (sqlite3_step(stmt) != SQLITE_ROW) goto step_fail;

    if (sqlite3_changes(db) == 0) return PLAYER_NOT_MODIFIED;

    in_out_player->id_player = sqlite3_column_int64(stmt, 0);
    strcpy(in_out_player->nickname, (const char*) sqlite3_column_text(stmt, 1));
    strcpy(in_out_player->email, (const char*) sqlite3_column_text(stmt, 2));
    strcpy(in_out_player->password, (const char*) sqlite3_column_text(stmt, 3));
    in_out_player->current_streak = sqlite3_column_int(stmt, 4);
    in_out_player->max_streak = sqlite3_column_int(stmt, 5);
    in_out_player->registration_date = (time_t) sqlite3_column_int64(stmt, 6);

    sqlite3_finalize(stmt);
    return PLAYER_OK;

    prepare_fail:
    fprintf(stderr, "DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return PLAYER_SQL_ERROR;

    bind_fail:
        fprintf(stderr, "DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAYER_SQL_ERROR;

    step_fail:
        fprintf(stderr, "DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return PLAYER_SQL_ERROR;
}