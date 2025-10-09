#include <string.h>
#include <time.h>

#include "participation_request_dto.h"

void map_participation_request_to_dto(const ParticipationRequest *pr, const char *player_nick, ParticipationRequestDTO *out_dto) {
    if (!pr || !out_dto) return;

    out_dto->id_request = pr->id_request;
    out_dto->id_game    = pr->id_game;

    // nickname
    strncpy(out_dto->player_nickname, player_nick ? player_nick : "", sizeof(out_dto->player_nickname));
    out_dto->player_nickname[sizeof(out_dto->player_nickname) - 1] = '\0';

    // state
    strncpy(out_dto->state_str, request_participation_status_to_string(pr->state), sizeof(out_dto->state_str));
    out_dto->state_str[sizeof(out_dto->state_str) - 1] = '\0';

    // created_at in ISO 8601 UTC format
    struct tm tm_info;
    if (gmtime_r(&pr->created_at, &tm_info)) {
        strftime(out_dto->created_at_str, DATE_STR_MAX, "%Y-%m-%dT%H:%MZ", &tm_info);
    } else {
        strncpy(out_dto->created_at_str, "invalid-date", DATE_STR_MAX);
        out_dto->created_at_str[DATE_STR_MAX - 1] = '\0';
    }
}
