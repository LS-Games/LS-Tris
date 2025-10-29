#ifndef PLAY_ENTITY_H
#define PLAY_ENTITY_H

#include <inttypes.h>

typedef enum {
    WIN,
    LOSE,
    DRAW,
    PLAY_RESULT_INVALID
} PlayResult;

typedef struct {
    int64_t id_player;
    int64_t id_round;
    PlayResult result;
    int player_number;
} Play;

void print_play(const Play *p);
void print_play_inline(const Play *p);

const char *play_result_to_string(PlayResult result);
PlayResult string_to_play_result(const char *result_str);

#endif