#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"

#include "./db/sqlite/db_connection_sqlite.h"
#include "./json-parser/test_json-parser.h"

#include "./controllers/round_controller.h"

int main(void) {

    // Open database
    sqlite3 *db = db_open();
    if (!db) {
        LOG_ERROR("%s","Failed to open the database.\n");
        return 1;
    }

    // Close database
    db_close(db);

    LOG_DEBUG("RoundStatus: %d\n", start_round(1, 500));
    LOG_DEBUG("RoundStatus: %d\n", end_round(0));

    // Parse json
    // json_c();
    
    return 0;
}