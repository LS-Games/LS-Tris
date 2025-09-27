#include <string.h>

#include "round_dto.h"

void map_round_to_dto(const Round *round, RoundDTO *out_dto) {
    if (!round || !out_dto) return;

    out_dto->id_round = round->id_round;
    out_dto->id_game  = round->id_game;
    out_dto->duration = round->duration;

    // state
    strncpy(out_dto->state_str, round_status_to_string(round->state), sizeof(out_dto->state_str));
    out_dto->state_str[sizeof(out_dto->state_str) - 1] = '\0';

    // board
    strncpy(out_dto->board, round->board, BOARD_MAX);
    out_dto->board[BOARD_MAX - 1] = '\0';
}