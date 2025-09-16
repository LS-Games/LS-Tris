#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <stdint.h>

#define DATE_STR_MAX 100

typedef struct GameDTO {
    int64_t id_game;
    char creator_nickname[64];
    char owner_nickname[64];
    char state_str[16];
    char created_at_str[DATE_STR_MAX];
} GameDTO;

#endif