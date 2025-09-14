#ifndef GAME_DAO_SQLITE_H
#define GAME_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/game_entity.h"
#include "../../db/sqlite/dto/game_join_player.h"

typedef enum {
    GAME_DAO_OK = 0,
    GAME_DAO_NOT_FOUND,
    GAME_DAO_SQL_ERROR,
    GAME_DAO_INVALID_INPUT,
    GAME_DAO_MALLOC_ERROR,
    GAME_DAO_NOT_MODIFIED
} GameReturnStatus;

typedef enum {
    UPDATE_GAME_ID_CREATOR     = 1 << 0,  
    UPDATE_GAME_ID_OWNER       = 1 << 1,  
    UPDATE_GAME_STATE          = 1 << 2,
    UPDATE_GAME_CREATED_AT     = 1 << 3  
} UpdateGameFlags;


// Funzioni CRUD concrete
GameReturnStatus get_game_by_id(sqlite3 *db, int64_t id_game, Game *out); 
GameReturnStatus get_all_games(sqlite3 *db, Game** out_array, int *out_count);
GameReturnStatus update_game_by_id(sqlite3 *db, const Game *upd_game);
GameReturnStatus delete_game_by_id(sqlite3 *db, int64_t id_game);
GameReturnStatus insert_game(sqlite3 *db, Game *in_out_game);

GameReturnStatus get_all_games_with_player_info(sqlite3 *db, GameWithPlayerNickname** out_array, int *out_count);

// Funzione di utilità per messaggi di errore
const char* return_game_status_to_string(GameReturnStatus status);

#endif