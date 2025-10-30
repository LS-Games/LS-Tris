#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"

#include "./server/server.h"

#include "./server/router.h"

#define SERVER_PORT 5050

int main(void) {

    int server_port = SERVER_PORT;
    LOG_INFO("%s\n", "Starting LS-TRIS server...");

    if (start_server(server_port) == 0) {

        LOG_INFO("Server started successfully on port %d\n", server_port);

    } else {

        LOG_ERROR("%s\n", "Failed to start server");
        exit(1);
    }

    return 0;
}