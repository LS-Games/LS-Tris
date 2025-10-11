#ifndef PARTICIPATION_REQUEST_DTO_H
#define PARTICIPATION_REQUEST_DTO_H

#include <stdint.h>

#include "../entities/participation_request_entity.h"

#define DATE_STR_MAX 100

typedef struct ParticipationRequestDTO {
    int64_t id_request;
    int64_t id_game;
    char player_nickname[64];
    char state_str[16];
    char created_at_str[DATE_STR_MAX];
} ParticipationRequestDTO;

void map_participation_request_to_dto(const ParticipationRequest *pr, const char *player_nick, ParticipationRequestDTO *out_dto);

#endif