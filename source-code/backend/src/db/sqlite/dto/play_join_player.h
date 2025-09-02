#ifndef PLAY_JOIN_PLAYER_H
#define PLAY_JOIN_PLAYER_H

#include "../../../entities/play_entity.h"
#include "../../../entities/player_entity.h"

typedef struct {
    int64_t id_player;
    int64_t id_round;
    PlayResult result;
    int player_number;
    char player_nickname[NICKNAME_MAX];
} PlayWithPlayerNickname;


#endif