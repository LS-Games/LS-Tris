#ifndef ROUND_DTO_H
#define ROUND_DTO_H

#include "../entities/round_entity.h"

typedef struct RoundDTO {
    int  id_round;        
    int  id_game;         
    char state_str[16];   
    int64_t duration;     
    char board[BOARD_MAX];
} RoundDTO;

#endif