#ifndef PARTICIPATION_REQUEST_DTO_H
#define PARTICIPATION_REQUEST_DTO_H

#include <stdint.h>

#define DATE_STR_MAX 100

typedef struct ParticipationRequestDTO {
    int64_t id_request;
    int64_t id_game;
    char player_nickname[64];
    char state_str[16];
    char created_at_str[DATE_STR_MAX];
} ParticipationRequestDTO;

#endif