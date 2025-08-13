#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H

#include <sqlite3.h>

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

typedef enum {
    PLAYER_OK = 0,
    PLAYER_NOT_FOUND,
    PLAYER_SQL_ERROR,
    PLAYER_INVALID_INPUT
} PlayerStatus;


PlayerStatus get_player_by_id(sqlite3 *db, int id, Player *out); //We use Player pointer parameter to work by reference rather than by value 
PlayerStatus get_all_players(sqlite3 *db, Player** out_array, int *out_count);
PlayerStatus update_player_by_id(sqlite3 *db, int id, const Player *in);
PlayerStatus delete_player_by_id(sqlite3 *db, int id);
PlayerStatus insert_player(sqlite3 *db, const Player *in);


#endif