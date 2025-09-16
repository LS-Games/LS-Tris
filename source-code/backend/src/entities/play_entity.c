#include <stdio.h>
#include <string.h>

#include "play_entity.h"

void print_play(const Play *p) {
    if (!p) {
        printf("Play: (NULL)\n");
        return;
    }

    printf("Play {\n");
    printf("  id_player: %" PRId64 "\n", p->id_player);
    printf("  id_round: %" PRId64 "\n", p->id_round);
    printf("  result: \"%s\"\n", play_result_to_string(p->result));
    printf("}\n");
}

void print_play_inline(const Play *p) {
    if (!p) {
        printf("Play(NULL)\n");
        return;
    }
    printf("Play[player=%" PRId64 ", round=%" PRId64 ", result=%s]\n",
            p->id_player,
            p->id_round,
            play_result_to_string(p->result));
}

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