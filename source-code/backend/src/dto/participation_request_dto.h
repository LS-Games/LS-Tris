#ifndef PARTICIPATION_REQUEST_DTO_H
#define PARTICIPATION_REQUEST_DTO_H

#include "../entities/participation_request_entity.h"

typedef struct ParticipationRequestDTO {
    int  id_request;            
    char player_nickname[64];   // Retrieved through id_player  
    int  id_game;               
    char created_at[DATE_MAX];  
    char state_str[16];         
} ParticipationRequestDTO;

#endif