#ifndef GAME_MODEL_H
#define GAME_MODEL_H

#include <sqlite3.h>

#define DATE_MAX 100

typedef enum {
    NEW_GAME,
    ACTIVE_GAME,
    WAITING_GAME,
    FINISHED_GAME,
    GAME_STATUS_INVALID
} GameStatus;

typedef struct Game {
    int id_game;
    int id_creator;
    int id_owner;
    GameStatus state;
    char created_at[DATE_MAX];
} Game;

typedef enum {
    GAME_OK = 0,
    GAME_NOT_FOUND,
    GAME_SQL_ERROR,
    GAME_INVALID_INPUT,
    GAME_MALLOC_ERROR,
    GAME_NOT_MODIFIED
} GameReturnStatus;

typedef enum {
    UPDATE_GAME_ID_CREATOR     = 1 << 0,  
    UPDATE_GAME_ID_OWNER       = 1 << 1,  
    UPDATE_GAME_STATE          = 1 << 2,
    UPDATE_GAME_CREATED_AT     = 1 << 3  
} UpdateGameFlags;

GameReturnStatus get_game_by_id(sqlite3 *db, int id_game, Game *out); 
GameReturnStatus get_all_games(sqlite3 *db, Game** out_array, int *out_count);
GameReturnStatus update_game_by_id(sqlite3 *db, const Game *upd_game);
GameReturnStatus delete_game_by_id(sqlite3 *db, int id_game);
GameReturnStatus insert_game(sqlite3 *db, const Game *in_game);
const char* return_game_status_to_string(GameReturnStatus status);
GameStatus string_to_game_status(const char *state_str);
const char* game_status_to_string(GameStatus state);

#endif