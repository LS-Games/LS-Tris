#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#define NICKNAME_MAX 100
#define MAIL_MAX 100
#define PASSWORD_MAX 100
#define DATE_MAX 100

#include <stdint.h>
#include <time.h>
typedef struct Player {
    int64_t id_player;
    char nickname[NICKNAME_MAX];
    char email[MAIL_MAX];
    char password[PASSWORD_MAX];
    int current_streak;
    int max_streak;
    time_t registration_date;
} Player;

void print_player(const Player *p);
void print_player_inline(const Player *p);

#endif