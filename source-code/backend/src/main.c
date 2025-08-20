#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./db/db_connection.h"

int main(void) {
    const char *db_path = "data/database.sqlite";
    sqlite3 *db = db_open(db_path);
    if (!db) {
        fprintf(stderr, "Impossibile aprire il DB.\n");
        return 1;
    }

    db_close(db);
    return 0;
}