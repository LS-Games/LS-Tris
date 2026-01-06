#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <stdint.h>

#include "../entities/game_entity.h"

#define DATE_STR_MAX 100

typedef struct GameDTO {
    int64_t id_game;
    char creator_nickname[64];
    char owner_nickname[64];
    int owner_current_streak;
    int owner_max_streak;
    char state_str[16];
    char created_at_str[DATE_STR_MAX];
} GameDTO;

void map_game_to_dto(
    const Game *game, 
    const char *creator_nick, 
    const char *owner_nick, 
    GameDTO *out_dto);

void map_game_with_streak_to_dto(
    const Game *game,
    const char *creator_nick,
    const char *owner_nick,
    int owner_current_streak,
    int owner_max_streak,
    GameDTO *out_dto
);

#endif