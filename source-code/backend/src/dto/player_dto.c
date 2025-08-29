#include <string.h>

#include "player_dto.h"

void map_player_to_dto(const Player *player, PlayerDTO *out_dto) {
    if (!player || !out_dto) return;

    out_dto->id_player      = player->id_player;
    out_dto->current_streak = player->current_streak;
    out_dto->max_streak     = player->max_streak;

    // nickname
    strncpy(out_dto->nickname, player->nickname, sizeof(out_dto->nickname));
    out_dto->nickname[sizeof(out_dto->nickname) - 1] = '\0';

    // registration_date
    strncpy(out_dto->registration_date, player->registration_date, DATE_MAX);
    out_dto->registration_date[DATE_MAX - 1] = '\0';
}