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
    PLAYER_INVALID_INPUT,
    PLAYER_MALLOC_ERROR,
    PLAYER_NOT_MODIFIED
} PlayerReturnStatus;

typedef enum {
    UPDATE_NICKNAME    = 1 << 0,  
    UPDATE_EMAIL       = 1 << 1,  
    UPDATE_PASSWORD    = 1 << 2,  
    UPDATE_CURRENT_STREAK = 1 << 3,
    UPDATE_MAX_STREAK     = 1 << 4,  
    UPDATE_REG_DATE       = 1 << 5   
} UpdatePlayerFlags;

PlayerReturnStatus get_player_by_id(sqlite3 *db, int id, Player *out); //We use Player pointer parameter to work by reference rather than by value 
PlayerReturnStatus get_all_players(sqlite3 *db, Player** out_array, int *out_count);
PlayerReturnStatus update_player_by_id(sqlite3 *db, const Player *upd_player);
PlayerReturnStatus delete_player_by_id(sqlite3 *db, int id);
PlayerReturnStatus insert_player(sqlite3 *db, const Player *in);
const char* player_status_to_string(PlayerReturnStatus status);


#endif