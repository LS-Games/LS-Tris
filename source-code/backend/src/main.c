#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "./models/player_model.h"
#include "./models/game_model.h"
#include "./db/db_connection.h"
#include "./models/participation_request_model.h"
#include "./models/play_model.h"
#include "./models/round_model.h"

static void divline(const char *title) {
    printf("\n==================== %s ====================\n", title);
}

static void print_round(const Round *r, const char *prefix) {
    printf("%s id_round=%d id_game=%d state=%s duration=%" PRId64 "s\n",
           prefix ? prefix : "",
           r->id_round, r->id_game,
           round_status_to_string(r->state),
           r->duration);
}

int main(void) {
    const char *db_path = "data/database.sqlite";
    sqlite3 *db = db_open(db_path);
    if (!db) {
        fprintf(stderr, "Impossibile aprire il DB.\n");
        return 1;
    }

    /* ==================== GET BY ID (ID esistenti 1..3 se presenti) ==================== */
    divline("ROUND get_by_id (1..3)");
    for (int id = 1; id <= 3; ++id) {
        Round r;
        RoundReturnStatus st = get_round_by_id(db, id, &r);
        printf("[get_by_id %d] -> %s\n", id, return_round_status_to_string(st));
        if (st == ROUND_OK) print_round(&r, "  ");
    }

    /* ==================== GET ALL ==================== */
    divline("ROUND get_all");
    Round *rarr = NULL;
    int rcount = 0;
    RoundReturnStatus rst = get_all_rounds(db, &rarr, &rcount);
    printf("[get_all] -> %s (count=%d)\n", return_round_status_to_string(rst), rcount);
    if (rst == ROUND_OK && rarr) {
        int show = rcount < 10 ? rcount : 10; // stampa al massimo i primi 10
        for (int i = 0; i < show; ++i) {
            print_round(&rarr[i], "  ");
        }
        free(rarr);
    }

    /* ==================== INSERT ==================== */
    divline("ROUND insert");
    Round new_r;
    memset(&new_r, 0, sizeof new_r);
    new_r.id_game  = 1;            // FK valida (1..3)
    new_r.state    = PENDING_ROUND;      // ACTIVE/PENDING/FINISHED
    new_r.duration = 120;          // secondi

    rst = insert_round(db, &new_r);
    printf("[insert] -> %s\n", return_round_status_to_string(rst));
    if (rst != ROUND_OK) { db_close(db); return 1; }

    int new_id = (int)sqlite3_last_insert_rowid(db);
    printf("  nuovo id_round inserito = %d\n", new_id);

    Round fetched;
    rst = get_round_by_id(db, new_id, &fetched);
    printf("[get_by_id %d dopo insert] -> %s\n", new_id, return_round_status_to_string(rst));
    if (rst == ROUND_OK) print_round(&fetched, "  ");

    /* ==================== UPDATE sulla riga appena inserita ==================== */
    divline("ROUND update (sulla riga inserita)");
    Round before = fetched;

    // Cambia alcuni campi mantenendo FK valide (1..3)
    fetched.id_game  = (before.id_game == 1 ? 2 : 1);
    fetched.state    = (before.state == FINISHED_ROUND ? PENDING : FINISHED_ROUND);
    fetched.duration = before.duration + 30;

    rst = update_round_by_id(db, &fetched);
    printf("[update id_round=%d] -> %s\n", new_id, return_round_status_to_string(rst));

    Round after;
    rst = get_round_by_id(db, new_id, &after);
    printf("[get_by_id %d dopo update] -> %s\n", new_id, return_round_status_to_string(rst));
    if (rst == ROUND_OK) print_round(&after, "  ");

    /* ==================== DELETE (pulizia) ==================== */
    // divline("ROUND delete (pulizia riga inserita)");
    // rst = delete_round_by_id(db, new_id);
    // printf("[delete id_round=%d] -> %s\n", new_id, return_round_status_to_string(rst));

    Round probe;
    rst = get_round_by_id(db, new_id, &probe);
    printf("[get_by_id %d dopo delete] -> %s (atteso: NOT_FOUND)\n",
           new_id, return_round_status_to_string(rst));

    /* ==================== TEST NOT_MODIFIED (se esiste id=1) ==================== */
    divline("ROUND not modified (se id=1 esiste)");
    Round r1;
    rst = get_round_by_id(db, 1, &r1);
    if (rst == ROUND_OK) {
        Round same = r1; // nessuna modifica
        rst = update_round_by_id(db, &same);
        printf("[update senza cambi id_round=1] -> %s (atteso: ROUND_NOT_MODIFIED)\n",
               return_round_status_to_string(rst));
    } else {
        printf("  (round id=1 non presente, salto test NOT_MODIFIED)\n");
    }

    /* ==================== TEST INPUT INVALIDI (rapidi) ==================== */
    divline("ROUND invalid inputs (smoke tests)");
    rst = get_round_by_id(db, 0, &probe);
    printf("[get_by_id id=0] -> %s (atteso: INVALID_INPUT)\n", return_round_status_to_string(rst));

    rst = delete_round_by_id(db, 0);
    printf("[delete id=0] -> %s (atteso: INVALID_INPUT)\n", return_round_status_to_string(rst));

    Round bad;
    memset(&bad, 0, sizeof bad);
    bad.id_game  = 0;                   // FK non valida
    bad.state    = ROUND_STATUS_INVALID; // stato non valido
    bad.duration = -1;                  // durata negativa
    rst = insert_round(db, &bad);
    printf("[insert dati invalidi] -> %s (atteso: INVALID_INPUT)\n", return_round_status_to_string(rst));

    divline("DONE");
    db_close(db);
    return 0;
}