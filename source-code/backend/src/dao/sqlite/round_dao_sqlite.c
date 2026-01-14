#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../../include/debug_log.h"
#include "round_dao_sqlite.h"

/* =========================================================
 * Utils
 * ========================================================= */

const char *return_round_dao_status_to_string(RoundDaoStatus status) {
    switch (status) {
        case ROUND_DAO_OK:              return "ROUND_DAO_OK";
        case ROUND_DAO_INVALID_INPUT:   return "ROUND_DAO_INVALID_INPUT";
        case ROUND_DAO_SQL_ERROR:       return "ROUND_DAO_SQL_ERROR";
        case ROUND_DAO_NOT_FOUND:       return "ROUND_DAO_NOT_FOUND";
        case ROUND_DAO_NOT_MODIFIED:    return "ROUND_DAO_NOT_MODIFIED";
        default:                        return "ROUND_DAO_UNKNOWN";
    }
}

/* =========================================================
 * GET ROUND BY ID
 * ========================================================= */

RoundDaoStatus get_round_by_id(sqlite3 *db, int64_t id_round, Round *out) {

    if (!db || !out || id_round <= 0)
        return ROUND_DAO_INVALID_INPUT;

    memset(out, 0, sizeof(*out));

    const char *sql =
        "SELECT id_round, id_game, state, start_time, end_time, board "
        "FROM Round WHERE id_round = ?1";

    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    rc = sqlite3_bind_int64(st, 1, id_round);
    if (rc != SQLITE_OK) goto bind_fail;

    rc = sqlite3_step(st);

    if (rc == SQLITE_ROW) {

        out->id_round   = sqlite3_column_int64(st, 0);
        out->id_game    = sqlite3_column_int64(st, 1);

        const unsigned char *state = sqlite3_column_text(st, 2);
        out->state = state
            ? string_to_round_status((const char *)state)
            : ROUND_STATUS_INVALID;

        out->start_time = sqlite3_column_int64(st, 3);

        if (sqlite3_column_type(st, 4) == SQLITE_NULL)
            out->end_time = 0;
        else
            out->end_time = sqlite3_column_int64(st, 4);

        const unsigned char *board = sqlite3_column_text(st, 5);
        if (board) {
            strncpy(out->board, (const char *)board, sizeof(out->board) - 1);
            out->board[sizeof(out->board) - 1] = '\0';
        }

        sqlite3_finalize(st);
        return ROUND_DAO_OK;
    }

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(st);
        return ROUND_DAO_NOT_FOUND;
    }

    goto step_fail;

prepare_fail:
    LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return ROUND_DAO_SQL_ERROR;

bind_fail:
    LOG_ERROR("DATABASE ERROR (bind): %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ROUND_DAO_SQL_ERROR;

step_fail:
    LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ROUND_DAO_SQL_ERROR;
}

/* =========================================================
 * GET ALL ROUNDS
 * ========================================================= */

RoundDaoStatus get_all_rounds(sqlite3 *db, Round **out_array, int *out_count) {

    if (!db || !out_array || !out_count)
        return ROUND_DAO_INVALID_INPUT;

    *out_array = NULL;
    *out_count = 0;

    const char *sql =
        "SELECT id_round, id_game, state, start_time, end_time, board FROM Round";

    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK) goto prepare_fail;

    int cap = 16;
    int count = 0;

    Round *array = malloc(sizeof(Round) * cap);
    if (!array) {
        sqlite3_finalize(st);
        return ROUND_DAO_MALLOC_ERROR;
    }

    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {

        if (count == cap) {
            cap *= 2;
            Round *tmp = realloc(array, sizeof(Round) * cap);
            if (!tmp) {
                free(array);
                sqlite3_finalize(st);
                return ROUND_DAO_MALLOC_ERROR;
            }
            array = tmp;
        }

        Round r = {0};

        r.id_round   = sqlite3_column_int64(st, 0);
        r.id_game    = sqlite3_column_int64(st, 1);

        const unsigned char *state = sqlite3_column_text(st, 2);
        r.state = state
            ? string_to_round_status((const char *)state)
            : ROUND_STATUS_INVALID;

        r.start_time = sqlite3_column_int64(st, 3);

        if (sqlite3_column_type(st, 4) == SQLITE_NULL)
            r.end_time = 0;
        else
            r.end_time = sqlite3_column_int64(st, 4);

        const unsigned char *board = sqlite3_column_text(st, 5);
        if (board) {
            strncpy(r.board, (const char *)board, sizeof(r.board) - 1);
            r.board[sizeof(r.board) - 1] = '\0';
        }

        array[count++] = r;
    }

    sqlite3_finalize(st);

    *out_array = array;
    *out_count = count;
    return ROUND_DAO_OK;

prepare_fail:
    LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return ROUND_DAO_SQL_ERROR;
}

/* =========================================================
 * INSERT ROUND
 * ========================================================= */

RoundDaoStatus insert_round(sqlite3 *db, Round *r) {

    if (!db || !r || r->id_game <= 0 || r->start_time <= 0)
        return ROUND_DAO_INVALID_INPUT;

    const char *sql =
        "INSERT INTO Round (id_game, state, start_time, end_time, board) "
        "VALUES (?1, ?2, ?3, NULL, ?4) RETURNING id_round";

    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK)
        goto prepare_fail;

    sqlite3_bind_int64(st, 1, r->id_game);
    sqlite3_bind_text(st, 2,
        round_status_to_string(r->state), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(st, 3, r->start_time);
    sqlite3_bind_text(st, 4, r->board, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(st) != SQLITE_ROW)
        goto step_fail;

    r->id_round = sqlite3_column_int64(st, 0);

    sqlite3_finalize(st);
    return ROUND_DAO_OK;

prepare_fail:
    LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return ROUND_DAO_SQL_ERROR;

step_fail:
    LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ROUND_DAO_SQL_ERROR;
}

/* =========================================================
 * UPDATE ROUND
 * ========================================================= */

RoundDaoStatus update_round_by_id(sqlite3 *db, const Round *upd) {

    if (!db || !upd || upd->id_round <= 0)
        return ROUND_DAO_INVALID_INPUT;

    Round orig = {0};
    RoundDaoStatus st = get_round_by_id(db, upd->id_round, &orig);
    if (st != ROUND_DAO_OK)
        return st;

    UpdateRoundFlags flags = 0;

    if (orig.state      != upd->state)      flags |= UPDATE_ROUND_STATE;
    if (orig.end_time   != upd->end_time)   flags |= UPDATE_ROUND_END_TIME;
    if (strcmp(orig.board, upd->board) != 0)
        flags |= UPDATE_ROUND_BOARD;

    if (flags == 0)
        return ROUND_DAO_NOT_MODIFIED;

    char query[256] = "UPDATE Round SET ";
    bool first = true;

    if (flags & UPDATE_ROUND_STATE) {
        strcat(query, "state = ?");
        first = false;
    }

    if (flags & UPDATE_ROUND_END_TIME) {
        if (!first) strcat(query, ", ");
        strcat(query, "end_time = ?");
        first = false;
    }

    if (flags & UPDATE_ROUND_BOARD) {
        if (!first) strcat(query, ", ");
        strcat(query, "board = ?");
    }

    strcat(query, " WHERE id_round = ?");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK)
        goto prepare_fail;

    int idx = 1;

    if (flags & UPDATE_ROUND_STATE)
        sqlite3_bind_text(stmt, idx++,
            round_status_to_string(upd->state), -1, SQLITE_TRANSIENT);

    if (flags & UPDATE_ROUND_END_TIME) {
        if (upd->end_time == 0)
            sqlite3_bind_null(stmt, idx++);
        else
            sqlite3_bind_int64(stmt, idx++, upd->end_time);
    }

    if (flags & UPDATE_ROUND_BOARD)
        sqlite3_bind_text(stmt, idx++, upd->board, -1, SQLITE_TRANSIENT);

    sqlite3_bind_int64(stmt, idx, upd->id_round);

    if (sqlite3_step(stmt) != SQLITE_DONE)
        goto step_fail;

    sqlite3_finalize(stmt);
    return sqlite3_changes(db) > 0
        ? ROUND_DAO_OK
        : ROUND_DAO_NOT_MODIFIED;

prepare_fail:
    LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return ROUND_DAO_SQL_ERROR;

step_fail:
    LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return ROUND_DAO_SQL_ERROR;
}

/* =========================================================
 * DELETE ROUND
 * ========================================================= */

RoundDaoStatus delete_round_by_id(sqlite3 *db, int64_t id_round) {

    if (!db || id_round <= 0)
        return ROUND_DAO_INVALID_INPUT;

    const char *sql = "DELETE FROM Round WHERE id_round = ?1";
    sqlite3_stmt *st = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK)
        goto prepare_fail;

    sqlite3_bind_int64(st, 1, id_round);

    if (sqlite3_step(st) != SQLITE_DONE)
        goto step_fail;

    sqlite3_finalize(st);
    return sqlite3_changes(db) > 0
        ? ROUND_DAO_OK
        : ROUND_DAO_NOT_FOUND;

prepare_fail:
    LOG_ERROR("DATABASE ERROR (prepare): %s\n", sqlite3_errmsg(db));
    return ROUND_DAO_SQL_ERROR;

step_fail:
    LOG_ERROR("DATABASE ERROR (step): %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(st);
    return ROUND_DAO_SQL_ERROR;
}

/* =========================================================
 * FULL ROUND INFO
 * ========================================================= */

RoundDaoStatus round_find_full_info(sqlite3 *db, int64_t id_round, RoundFullDTO *out) {

    if (!db || !out)
        return ROUND_DAO_INVALID_INPUT;

    memset(out, 0, sizeof(*out));

    const char *sql =
        "SELECT "
        "r.id_round, r.id_game, r.state, r.start_time, r.end_time, r.board, "
        "p1.id_player, pl1.player_number, p1.nickname, "
        "p2.id_player, pl2.player_number, p2.nickname "
        "FROM Round r "
        "JOIN Play pl1 ON pl1.id_round = r.id_round AND pl1.player_number = 1 "
        "JOIN Player p1 ON p1.id_player = pl1.id_player "
        "JOIN Play pl2 ON pl2.id_round = r.id_round AND pl2.player_number = 2 "
        "JOIN Player p2 ON p2.id_player = pl2.id_player "
        "WHERE r.id_round = ?1";

    sqlite3_stmt *st = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK)
        return ROUND_DAO_SQL_ERROR;

    sqlite3_bind_int64(st, 1, id_round);

    if (sqlite3_step(st) != SQLITE_ROW) {
        sqlite3_finalize(st);
        return ROUND_DAO_NOT_FOUND;
    }

    out->id_round   = sqlite3_column_int64(st, 0);
    out->id_game    = sqlite3_column_int64(st, 1);

    const unsigned char *state = sqlite3_column_text(st, 2);
    strncpy(out->state, state ? (const char *)state : "",
            sizeof(out->state) - 1);

    out->start_time = sqlite3_column_int64(st, 3);

    if (sqlite3_column_type(st, 4) == SQLITE_NULL)
        out->end_time = 0;
    else
        out->end_time = sqlite3_column_int64(st, 4);

    const unsigned char *board = sqlite3_column_text(st, 5);
    strncpy(out->board, board ? (const char *)board : "",
            sizeof(out->board) - 1);

    out->id_player1 = sqlite3_column_int64(st, 6);
    out->player_number_player1 = sqlite3_column_int(st, 7);

    const unsigned char *n1 = sqlite3_column_text(st, 8);
    strncpy(out->nickname_player1, n1 ? (const char *)n1 : "",
            sizeof(out->nickname_player1) - 1);

    out->id_player2 = sqlite3_column_int64(st, 9);
    out->player_number_player2 = sqlite3_column_int(st, 10);

    const unsigned char *n2 = sqlite3_column_text(st, 11);
    strncpy(out->nickname_player2, n2 ? (const char *)n2 : "",
            sizeof(out->nickname_player2) - 1);

    sqlite3_finalize(st);
    return ROUND_DAO_OK;
}
