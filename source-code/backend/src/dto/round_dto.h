#ifndef ROUND_DTO_H
#define ROUND_DTO_H

#include "../entities/round_entity.h"

#include <stdint.h>

#define ROUND_STATE_STR_MAX 16

typedef struct RoundDTO {
    int64_t id_round;
    int64_t id_game;
    int64_t duration;
    char state_str[ROUND_STATE_STR_MAX];
    char board[BOARD_MAX];
} RoundDTO;

void map_round_to_dto(const Round *round, RoundDTO *out_dto);

#endif