#ifndef PLAYER_DAO_SQLITE_H
#define PLAYER_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/player_entity.h"

typedef enum {
    PLAYER_DAO_OK = 0,
    PLAYER_DAO_NOT_FOUND,
    PLAYER_DAO_SQL_ERROR,
    PLAYER_DAO_INVALID_INPUT,
    PLAYER_DAO_MALLOC_ERROR,
    PLAYER_DAO_NOT_MODIFIED
} PlayerReturnStatus;

typedef enum {
    UPDATE_PLAYER_NICKNAME          = 1 << 0,  
    UPDATE_PLAYER_EMAIL             = 1 << 1,  
    UPDATE_PLAYER_PASSWORD          = 1 << 2,  
    UPDATE_PLAYER_CURRENT_STREAK    = 1 << 3,
    UPDATE_PLAYER_MAX_STREAK        = 1 << 4,  
    UPDATE_PLAYER_REG_DATE          = 1 << 5   
} UpdatePlayerFlags;


// Funzioni CRUD concrete
PlayerReturnStatus get_player_by_id(sqlite3 *db, int64_t id_player, Player *out); //We use Player pointer parameter to work by reference rather than by value 
PlayerReturnStatus get_all_players(sqlite3 *db, Player** out_array, int *out_count);
PlayerReturnStatus update_player_by_id(sqlite3 *db, const Player *upd_player);
PlayerReturnStatus delete_player_by_id(sqlite3 *db, int64_t id_player);
PlayerReturnStatus insert_player(sqlite3 *db, Player *in_out_player);

// Funzione di utilità per messaggi di errore
const char* return_player_status_to_string(PlayerReturnStatus status);

#endif