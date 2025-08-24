#ifndef ROUND_MODEL_H
#define ROUND_MODEL_H

#include <sqlite3.h>
#include <stdint.h>

#define BOARD_MAX 12

typedef enum {
    ACTIVE_ROUND,
    PENDING_ROUND,
    FINISHED_ROUND,
    ROUND_STATUS_INVALID
} RoundStatus;

typedef struct {
    int id_round;
    int id_game;
    RoundStatus state;
    int64_t duration;
    char board[BOARD_MAX];
} Round;

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

RoundReturnStatus get_round_by_id(sqlite3 *db, int id_round, Round *out); 
RoundReturnStatus get_all_rounds(sqlite3 *db, Round** out_array, int *out_count);
RoundReturnStatus update_round_by_id(sqlite3 *db, const Round *upd_round);
RoundReturnStatus delete_round_by_id(sqlite3 *db, int id_round);
RoundReturnStatus insert_round(sqlite3 *db, const Round *in_round);
const char* return_round_status_to_string(RoundReturnStatus status);
RoundStatus string_to_round_status(const char *state_str);
const char* round_status_to_string(RoundStatus state);


#endif