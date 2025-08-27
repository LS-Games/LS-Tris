#include <stdio.h>
#include <string.h>

#include "play_entity.h"

const char* play_result_to_string(PlayResult result) {
    switch (result) {
        case WIN :              return "win";
        case LOSE :             return "lose";
        case DRAW :             return "draw";
        default:                return NULL;
    }
}

PlayResult string_to_play_result(const char *result_str) {
    if (result_str) {
        if (strcmp(result_str, "win") == 0)     return WIN;
        if (strcmp(result_str, "lose") == 0)    return LOSE;
        if (strcmp(result_str, "draw") == 0)    return DRAW;
    }

    return PLAY_RESULT_INVALID; 
}