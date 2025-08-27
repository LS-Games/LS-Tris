#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#define NICKNAME_MAX 100
#define MAIL_MAX 100
#define PASSWORD_MAX 100
#define DATE_MAX 100

typedef struct Player {
    int id_player;
    char nickname[NICKNAME_MAX];
    char email[MAIL_MAX];
    char password[PASSWORD_MAX];
    int current_streak;
    int max_streak;
    char registration_date[DATE_MAX];
} Player;

#endif