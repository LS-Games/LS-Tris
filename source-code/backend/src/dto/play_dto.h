#ifndef PLAY_DTO_H
#define PLAY_DTO_H

#include <stdint.h>

#define RESULT_STR_MAX 8

typedef struct PlayDTO {
    int64_t id_player;
    int64_t id_round;
    int     player_number;
    char    player_nickname[64];
    char    result_str[RESULT_STR_MAX];
} PlayDTO;

#endif