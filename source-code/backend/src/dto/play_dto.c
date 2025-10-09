#include <string.h>

#include "play_dto.h"

void map_play_to_dto(const Play *play, const char *player_nick, PlayDTO *out_dto) {
    if (!play || !out_dto) return;

    out_dto->id_player     = play->id_player;
    out_dto->id_round      = play->id_round;
    out_dto->player_number = play->player_number;

    // nickname
    strncpy(out_dto->player_nickname, player_nick ? player_nick : "", sizeof(out_dto->player_nickname));
    out_dto->player_nickname[sizeof(out_dto->player_nickname) - 1] = '\0';

    // result
    strncpy(out_dto->result_str, play_result_to_string(play->result), sizeof(out_dto->result_str));
    out_dto->result_str[sizeof(out_dto->result_str) - 1] = '\0';
}
