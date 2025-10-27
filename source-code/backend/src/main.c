#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"
#include "./server/server.h"

#include "./dao/sqlite/db_connection_sqlite.h"
#include "./json-parser/test_json-parser.h"

#include "./dto/player_dto.h"
#include "./dto/game_dto.h"
#include "./controllers/player_controller.h"
#include "./controllers/game_controller.h"

#define SERVER_PORT 5050

int main(void) {

    int server_port = SERVER_PORT;
    LOG_INFO("Starting LS-TRIS server...\n");

    if (start_server(server_port) == 0) {

        LOG_INFO("Server started successfully on port %d\n", server_port);

    } else {

        LOG_ERROR("Failed to start server\n");
        return 1;
    }

    return 0;
}