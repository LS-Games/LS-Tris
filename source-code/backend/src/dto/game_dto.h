#ifndef GAME_DTO_H
#define GAME_DTO_H

#include "../entities/game_entity.h"

typedef struct GameDTO {
    int  id_game;
    char creator_nickname[64];  // Retrieved through id_creator
    char owner_nickname[64];    // Retrieved through id_owner
    char state_str[16];
    char created_at[DATE_MAX];
} GameDTO;

#endif