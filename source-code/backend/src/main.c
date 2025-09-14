#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"

#include "./db/sqlite/db_connection_sqlite.h"
#include "./json-parser/test_json-parser.h"

#include "./controllers/round_controller.h"
#include "./entities/participation_request_entity.h"

int main(void) {

    ParticipationRequest p = {
        .id_game = 1,
        .id_player = 1,
        .id_request = 1,
        .state = PENDING,
        .created_at = time(NULL)
    };

    print_participation_request(&p);

    // Parse json
    // json_c();
    
    return 0;
}