#ifndef PLAY_DAO_SQLITE_H
#define PLAY_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/play_entity.h"
#include "../dto/play_join_player.h"

typedef enum {
    PLAY_DAO_OK = 0,
    PLAY_DAO_NOT_FOUND,
    PLAY_DAO_SQL_ERROR,
    PLAY_DAO_INVALID_INPUT,
    PLAY_DAO_MALLOC_ERROR,
    PLAY_DAO_NOT_MODIFIED
} PlayDaoStatus;

typedef enum {
    UPDATE_PLAY_RESULT           = 1 << 0,
    UPDATE_PLAY_PLAYER_NUMBER    = 1 << 1
} UpdatePlayFlags;


// Funzioni CRUD concrete
PlayDaoStatus get_play_by_pk(sqlite3 *db, int64_t id_player, int64_t id_round, Play *out); 
PlayDaoStatus get_all_plays(sqlite3 *db, Play** out_array, int *out_count);
PlayDaoStatus update_play_by_pk(sqlite3 *db, Play *upd_play);
PlayDaoStatus delete_play_by_pk(sqlite3 *db, int64_t id_play, int64_t id_round);
PlayDaoStatus insert_play(sqlite3 *db, Play *in_out_play);

PlayDaoStatus get_all_plays_by_round(sqlite3 *db, Play** out_array, int64_t id_round, int *out_count);
PlayDaoStatus get_all_plays_with_player_info(sqlite3 *db, PlayWithPlayerNickname** out_array, int *out_count);

// Funzione di utilità per messaggi di errore
const char *return_play_dao_status_to_string(PlayDaoStatus status);

#endif