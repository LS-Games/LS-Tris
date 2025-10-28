#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <sqlite3.h>

// Declare .sqlite file path
#define DB_PATH "../db/data/database.sqlite"

sqlite3* db_open();
void db_close(sqlite3* db);

#endif