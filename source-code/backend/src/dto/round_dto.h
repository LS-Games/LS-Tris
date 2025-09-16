#ifndef ROUND_DTO_H
#define ROUND_DTO_H

#include <stdint.h>
#include "../entities/round_entity.h"

#define ROUND_STATE_STR_MAX 16

typedef struct RoundDTO {
    int64_t id_round;
    int64_t id_game;
    int64_t duration;
    char state_str[ROUND_STATE_STR_MAX];
    char board[BOARD_MAX];
} RoundDTO;

#endif