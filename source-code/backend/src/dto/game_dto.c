#include <string.h>
#include <time.h>

#include "game_dto.h"
#include "../entities/game_entity.h"

void map_game_to_dto(const Game *game, const char *creator_nick, const char *owner_nick, GameDTO *out_dto) {
    if (!game || !out_dto) return;

    out_dto->id_game = game->id_game;

    // creator_nickname
    strncpy(out_dto->creator_nickname, creator_nick ? creator_nick : "", sizeof(out_dto->creator_nickname));
    out_dto->creator_nickname[sizeof(out_dto->creator_nickname) - 1] = '\0';

    // owner_nickname
    strncpy(out_dto->owner_nickname, owner_nick ? owner_nick : "", sizeof(out_dto->owner_nickname));
    out_dto->owner_nickname[sizeof(out_dto->owner_nickname) - 1] = '\0';

    // state
    strncpy(out_dto->state_str, game_status_to_string(game->state), sizeof(out_dto->state_str));
    out_dto->state_str[sizeof(out_dto->state_str) - 1] = '\0';

    // created_at in ISO 8601 UTC format
    struct tm tm_info;
    if (gmtime_r(&game->created_at, &tm_info)) {
        strftime(out_dto->created_at_str, DATE_STR_MAX, "%Y-%m-%dT%H:%MZ", &tm_info);
    } else {
        strncpy(out_dto->created_at_str, "invalid-date", DATE_STR_MAX);
        out_dto->created_at_str[DATE_STR_MAX - 1] = '\0';
    }
}
