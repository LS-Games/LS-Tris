#include <stdio.h>
#include <string.h>

#include "game_entity.h"

void print_game(const Game *g) {
    if (!g) {
        printf("Game: (NULL)\n");
        return;
    }

    printf("Game {\n");
    printf("  id_game: %d\n", g->id_game);
    printf("  id_creator: %d\n", g->id_creator);
    printf("  id_owner: %d\n", g->id_owner);
    printf("  state: \"%s\"\n", game_status_to_string(g->state));
    printf("  created_at: \"%s\"\n", g->created_at);
    printf("}\n");
}

const char* game_status_to_string(GameStatus state) {
    switch (state) {
        case NEW_GAME :         return "new";
        case ACTIVE_GAME :      return "active";
        case WAITING_GAME :     return "waiting";
        case FINISHED_GAME :    return "finished";    
        default:                return NULL;
    }
}

void print_game_inline(const Game *g) {
    if (!g) {
        printf("Game(NULL)\n");
        return;
    }
    printf("Game[id=%d, creator=%d, owner=%d, state=%s, created_at=%s]\n",
            g->id_game,
            g->id_creator,
            g->id_owner,
            game_status_to_string(g->state),
            g->created_at);
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