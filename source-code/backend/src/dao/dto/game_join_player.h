#ifndef GAME_JOIN_PLAYER_H
#define GAME_JOIN_PLAYER_H

#include "../../entities/game_entity.h"
#include "../../entities/player_entity.h"

typedef struct {
    int64_t id_game;
    int64_t id_creator;
    int64_t id_owner;

    GameStatus state;
    time_t created_at;

    char creator[NICKNAME_MAX];
    char owner[NICKNAME_MAX];

    int owner_current_streak;
    int owner_max_streak;
} GameWithPlayerNickname;

#endif