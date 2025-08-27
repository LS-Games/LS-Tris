#ifndef ROUND_DAO_SQLITE_H
#define ROUND_DAO_SQLITE_H

#include <sqlite3.h>

#include "../../entities/round_entity.h"

typedef enum {
    ROUND_OK = 0,
    ROUND_NOT_FOUND,
    ROUND_SQL_ERROR,
    ROUND_INVALID_INPUT,
    ROUND_MALLOC_ERROR,
    ROUND_NOT_MODIFIED
} RoundReturnStatus;

typedef enum {
    UPDATE_ROUND_ID_GAME        = 1 << 0,
    UPDATE_ROUND_STATE          = 1 << 1,
    UPDATE_ROUND_DURATION       = 1 << 2,
    UPDATE_ROUND_BOARD          = 1 << 3
} UpdateRoundFlags;


// Funzioni CRUD concrete
RoundReturnStatus get_round_by_id(sqlite3 *db, int id_round, Round *out); 
RoundReturnStatus get_all_rounds(sqlite3 *db, Round** out_array, int *out_count);
RoundReturnStatus update_round_by_id(sqlite3 *db, const Round *upd_round);
RoundReturnStatus delete_round_by_id(sqlite3 *db, int id_round);
RoundReturnStatus insert_round(sqlite3 *db, const Round *in_round);

// Funzione di utilità per messaggi di errore
const char* return_round_status_to_string(RoundReturnStatus status);

#endif