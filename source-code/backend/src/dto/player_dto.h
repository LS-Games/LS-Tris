#ifndef PLAYER_DTO_H
#define PLAYER_DTO_H

#include "../entities/player_entity.h"

typedef struct PlayerDTO {
    int  id_player;                  
    char nickname[NICKNAME_MAX];     
    int  current_streak;             
    int  max_streak;                 
    char registration_date[DATE_MAX];
} PlayerDTO;

#endif