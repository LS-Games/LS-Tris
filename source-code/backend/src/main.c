#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"

#include "./db/sqlite/db_connection_sqlite.h"
#include "./json-parser/test_json-parser.h"

#include "./controllers/player_controller.h"

int main(void) {

    Player* players;
    int n;
    player_find_all(&players, &n);
    for (int i=0; i<n; i++)
        LOG_STRUCT_DEBUG(print_player_inline, &(players[i]));


    player_signup("pippo", "pippo@gmail.com", "fratm");

    player_find_all(&players, &n);
    for (int i=0; i<n; i++)
        LOG_STRUCT_DEBUG(print_player_inline, &(players[i]));

    // Parse json
    // json_c();
    
    return 0;
}