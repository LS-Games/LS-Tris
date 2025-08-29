#include <stdio.h>

#include "../../../include/debug_log.h"

#include "db_connection_sqlite.h"

sqlite3* db_open() {

    sqlite3* db = NULL;

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
        }
    
        LOG_DEBUG("The database has been opened successfully: \"%s\"\n", DB_PATH);
    }

    return db;
}

void db_close(sqlite3* db) {

    if (db) {
        sqlite3_close(db);
        LOG_DEBUG("%s","Database closed.\n");
    }
}