#ifndef PLAYER_DTO_H
#define PLAYER_DTO_H

#include <stdint.h>

#define NICKNAME_MAX 100
#define DATE_STR_MAX 100

typedef struct PlayerDTO {
    int64_t id_player;
    char nickname[NICKNAME_MAX];
    int current_streak;
    int max_streak;
    char registration_date_str[DATE_STR_MAX];
} PlayerDTO;

#endif