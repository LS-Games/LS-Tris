#include <string.h>
#include <time.h>

#include "player_dto.h"
#include "../entities/player_entity.h"

void map_player_to_dto(const Player *player, PlayerDTO *out_dto)
{
    if (!player || !out_dto) return;

    out_dto->id_player      = player->id_player;
    out_dto->current_streak = player->current_streak;
    out_dto->max_streak     = player->max_streak;

    // nickname
    strncpy(out_dto->nickname, player->nickname, sizeof(out_dto->nickname));
    out_dto->nickname[sizeof(out_dto->nickname) - 1] = '\0';

    // registration_date in ISO 8601 UTC format
    struct tm tm_info;
    if (gmtime_r(&player->registration_date, &tm_info)) {
        strftime(out_dto->registration_date_str, DATE_STR_MAX, "%Y-%m-%dT%H:%MZ", &tm_info);
    } else {
        strncpy(out_dto->registration_date_str, "invalid-date", DATE_STR_MAX);
        out_dto->registration_date_str[DATE_STR_MAX - 1] = '\0';
    }
}