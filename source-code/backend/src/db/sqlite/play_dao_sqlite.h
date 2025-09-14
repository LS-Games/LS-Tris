#ifndef PLAY_DAO_SQLITE_H
#define PLAY_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/play_entity.h"
#include "../../db/sqlite/dto/play_join_player.h"

typedef enum {
    PLAY_OK = 0,
    PLAY_NOT_FOUND,
    PLAY_SQL_ERROR,
    PLAY_INVALID_INPUT,
    PLAY_MALLOC_ERROR,
    PLAY_NOT_MODIFIED
} PlayReturnStatus;

typedef enum {
    UPDATE_PLAY_RESULT           = 1 << 0,
    UPDATE_PLAY_PLAYER_NUMBER    = 1 << 1
} UpdatePlayFlags;


// Funzioni CRUD concrete
PlayReturnStatus get_play_by_pk(sqlite3 *db, int64_t id_player, int64_t id_round, Play *out); 
PlayReturnStatus get_all_plays(sqlite3 *db, Play** out_array, int *out_count);
PlayReturnStatus update_play_by_pk(sqlite3 *db, Play *upd_play);
PlayReturnStatus delete_play_by_pk(sqlite3 *db, int64_t id_play, int64_t id_round);
PlayReturnStatus insert_play(sqlite3 *db, Play *in_out_play);

PlayReturnStatus get_all_plays_by_round(sqlite3 *db, Play** out_array, int64_t id_round, int *out_count);
PlayReturnStatus get_all_plays_with_player_info(sqlite3 *db, PlayWithPlayerNickname** out_array, int *out_count);

// Funzione di utilità per messaggi di errore
const char* return_play_status_to_string(PlayReturnStatus status);

#endif