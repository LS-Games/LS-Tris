#include <stdio.h>
#include <string.h>

#include "game_entity.h"

const char* game_status_to_string(GameStatus state) {
    switch (state) {
        case NEW_GAME :         return "new";
        case ACTIVE_GAME :      return "active";
        case WAITING_GAME :     return "waiting";
        case FINISHED_GAME :    return "finished";    
        default:                return NULL;
    }
}

GameStatus string_to_game_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "new") == 0)          return NEW_GAME;
        if (strcmp(state_str, "active") == 0)       return ACTIVE_GAME;
        if (strcmp(state_str, "waiting") == 0)      return WAITING_GAME;
        if (strcmp(state_str, "finished") == 0)     return FINISHED_GAME;
    }

    return GAME_STATUS_INVALID; 
}