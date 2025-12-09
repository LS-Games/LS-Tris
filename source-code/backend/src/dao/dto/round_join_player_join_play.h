#ifndef ROUND_JOIN_PLAYER_JOIN_PLAY
#define ROUND_JOIN_PLAYER_JOIN_PLAY

#include <stdint.h>

#include "../../entities/round_entity.h"  
#include "../../entities/player_entity.h"  
#include "../../entities/play_entity.h"  

typedef struct RoundFullDTO {
    int64_t id_round;
    int64_t id_game;
    int64_t duration;
    char state[16];
    char board[BOARD_MAX];

    int64_t id_player1;
    int64_t id_player2;

    char nickname_player1[NICKNAME_MAX];
    char nickname_player2[NICKNAME_MAX];

    int player_number_player1;
    int player_number_player2;
} RoundFullDTO;

#endif