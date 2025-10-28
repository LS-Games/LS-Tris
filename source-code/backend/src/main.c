#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/debug_log.h"

#include "./json-parser/test_json-parser.h"
#include "./server/router.h"

int main(void) {

    route_request("{\"action\":\"player_get_public_info\",\"nickname\":\"pippo_nuovo\"}", 1);

    return 0;
}