#ifndef PLAY_DTO_H
#define PLAY_DTO_H

#include "../entities/play_entity.h"

typedef struct PlayDTO {
    int  id_player;
    char player_nickname[64];   // Retrieved through id_player
    int  id_round;            
    char result_str[8];       
} PlayDTO;

#endif