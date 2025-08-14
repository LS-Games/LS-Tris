#include <stdio.h>
#include <string.h>
#include "player_model.h"

PlayerStatus get_player_by_id(sqlite3 *db, int id, Player *out) {

    if(db == NULL || id <= 0 || out == NULL) {
        return PLAYER_INVALID_INPUT;
    }

    const char *sql = 
        "SELECT id_player, nickname, email, password, current_streak, max_streak, registration_date "
        "FROM Player WHERE id_player = ?1";

    sqlite3_stmt *st = NULL;    //pointer to compiled query (statement)

    //This SQLite function requires:
    //  1 - database 
    //  2 - query SQL (that we saved in sql)
    //  3 - query length (with -1 it calculates it in automatically)
    //  4 - in this pointer the function saved the address of compiled object, the latter is obtained from the translation of the SQL string into bytecode for the SQLite engine.
    //  5 - used to find out where the query part used has ended up (is optional)

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);

    if (rc != SQLITE_OK) return PLAYER_SQL_ERROR;

    //This SQLite function used to replace placeholder in the query
    //In our case it replaces placeholder "1" with id value

    rc = sqlite3_bind_int(st , 1, id);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
    }


    //This SQLite function executes the statement
    //In our case, we are using SELECT so the function should return a SQLITE_ROW if it finds a row

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_player = sqlite3_column_int(st, 0); //It takes the value from column with index 0 and assigns it to id_player
        const unsigned char *nickname = sqlite3_column_text(st, 1); //The function returns a const unsigned char
        const unsigned char *email = sqlite3_column_text(st, 2);
        const unsigned char *password = sqlite3_column_text(st, 3);
        out->current_streak = sqlite3_column_int(st, 4);
        out->max_streak = sqlite3_column_int(st, 5);
        const unsigned char *registration_date = sqlite3_column_text(st, 6);

        //Now we must save all the value returned by sqlite3_column_text function because they will be deleted when the statement closes
        //We cannot use the simple assignment operator (=) because the returned value is a pointer whereas the object fields are arrays of characters
        //We can use the strcpy() function to do it

        if (nickname) {
            strcpy(out->nickname, (const char*) nickname); //We do the cast because strcpy expects a const  
        } else {
            out->nickname[0] = '\0'; //So we avoid error when the returned value is NULL, ('\0' = " ")
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

        if (registration_date) {
            strcpy(out->registration_date, (const char*) registration_date);
        } else {
            out->registration_date[0] = '\0';
        }

        sqlite3_finalize(st); //We're closing the statement here
        return PLAYER_OK;

    } else if (rc == SQLITE_DONE) {

        sqlite3_finalize(st);
        return PLAYER_NOT_FOUND;

    } else {

        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
    }    
}

PlayerStatus get_all_players(sqlite3 *db, Player **out_array, int *out_count) {

    if(db == NULL || out_array == NULL || out_count == NULL) { //We only check if pointers are null
        return PLAYER_INVALID_INPUT;
    }

    *out_array = NULL;
    *out_count = 0; 

    const char *sql = "SELECT id_player, nickname, email, password, current_streak, max_streak, registration_date FROM Player"; //Avoid using SELECT *, so if the table will change we won't have problems with columns

    sqlite3_stmt *st = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);

    if (rc != SQLITE_OK) return PLAYER_SQL_ERROR;

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

        p.id_player = sqlite3_column_int(st,0);
        const unsigned char *nickname = sqlite3_column_text(st, 1);
        const unsigned char *email = sqlite3_column_text(st, 2);
        const unsigned char *password = sqlite3_column_text(st, 3);
        p.current_streak = sqlite3_column_int(st,4);
        p.max_streak = sqlite3_column_int(st,5);
        const unsigned char *registration_date = sqlite3_column_text(st,6);

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

        if(registration_date) {
            strcpy(p.registration_date, (const char*) registration_date);
        } else {
            p.registration_date[0] = '\0';
        }

        player_array[count++] = p;
    }

    if (rc != SQLITE_DONE) {
        free(player_array);
        sqlite3_finalize(st);
        return PLAYER_SQL_ERROR;
    }

    *out_array = player_array; //We're assigning to the caller pointer the address of array 
    *out_count = count;

    sqlite3_finalize(st);

    return PLAYER_OK;
}