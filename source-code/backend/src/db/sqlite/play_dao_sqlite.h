#ifndef PLAY_DAO_SQLITE_H
#define PLAY_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/play_entity.h"

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
} UpdatePlayFlags;


// Funzioni CRUD concrete
PlayReturnStatus get_play_by_pk(sqlite3 *db, int id_player, int id_round, Play *out); 
PlayReturnStatus get_all_plays(sqlite3 *db, Play** out_array, int *out_count);
PlayReturnStatus update_play_by_pk(sqlite3 *db, const Play *upd_play);
PlayReturnStatus delete_play_by_pk(sqlite3 *db, int id_play, int id_round);
PlayReturnStatus insert_play(sqlite3 *db, const Play *in_play);

// Funzione di utilità per messaggi di errore
const char* return_play_status_to_string(PlayReturnStatus status);

#endif