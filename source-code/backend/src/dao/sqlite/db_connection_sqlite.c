#include <stdio.h>
#include <unistd.h>

#include "../../../include/debug_log.h"

#include "db_connection_sqlite.h"

/**
 * Function that opens database 
 * @return A `sqlite3*` pointer that is ready to use. `NULL` if something goes wrong.
 */
sqlite3* db_open() {

    sqlite3* db = NULL;

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    LOG_DEBUG("Current working directory: %s\n", cwd);
    LOG_DEBUG("Trying to open DB at: %s\n", DB_PATH);

    //If an error occured in database opening we print the error and close it, even if it was partially opened
    if(sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        LOG_ERROR("Error occurred in database opening: \"%s\"\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
    } else {
        //Even if we have actived it in scheme.sql, every time the db closes the constraints return OFF
        if(sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0) != SQLITE_OK) {
            LOG_ERROR("Error occured during foreign_keys contraints activation: \"%s\"\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            db = NULL;
        } else {
            LOG_DEBUG("The database has been opened successfully: \"%s\"\n", DB_PATH);
        }
    }

    return db;
}

void db_close(sqlite3* db) {

    if (db) {
        sqlite3_close(db);
        LOG_DEBUG("%s\n","Database closed.");
    }
}