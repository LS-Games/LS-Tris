#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./db/sqlite/db_connection_sqlite.h"
#include "./json-parser/test_json-parser.h"

int main(void) {

    // Declare .sqlite file path
    const char *db_path = "./db/data/database.sqlite";

    // Open database
    sqlite3 *db = db_open(db_path);
    if (!db) {
        fprintf(stderr, "Failed to open the database.\n");
        return 1;
    }

    // Close database
    db_close(db);

    // Parse json
    json_c();

    return 0;
}