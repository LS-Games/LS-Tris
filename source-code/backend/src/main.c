#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./models/player_model.h"
#include "./models/game_model.h"
#include "./db/db_connection.h"
#include "./models/participation_request_model.h"
#include "./models/play_model.h"

static void divline(const char *title) {
    printf("\n==================== %s ====================\n", title);
}

static void print_play(const Play *p, const char *prefix) {
    printf("%s player=%d round=%d result=%s\n",
           prefix ? prefix : "",
           p->id_player, p->id_round, play_result_to_string(p->result));
}

int main(void) {
    const char *db_path = "data/database.sqlite";
    sqlite3 *db = db_open(db_path);
    if (!db) {
        fprintf(stderr, "Impossibile aprire il DB.\n");
        return 1;
    }

    /* ==================== GET BY PK (player 1..3, round 1..3) ==================== */
    divline("PLAY get_by_pk (player 1..3, round 1..3)");
    for (int pid = 1; pid <= 3; ++pid) {
        for (int rid = 1; rid <= 3; ++rid) {
            Play p;
            PlayReturnStatus st = get_play_by_pk(db, pid, rid, &p);
            printf("[get_by_pk p=%d r=%d] -> %s\n",
                   pid, rid, return_play_status_to_string(st));
            if (st == PLAY_OK) print_play(&p, "  ");
        }
    }

    /* ==================== GET ALL ==================== */
    divline("PLAY get_all");
    Play *parr = NULL;
    int pcount = 0;
    PlayReturnStatus pst = get_all_plays(db, &parr, &pcount);
    printf("[get_all] -> %s (count=%d)\n", return_play_status_to_string(pst), pcount);
    if (pst == PLAY_OK && parr) {
        int show = pcount < 10 ? pcount : 10; // stampa max i primi 10
        for (int i = 0; i < show; ++i) {
            print_play(&parr[i], "  ");
        }
        free(parr);
    }

    /* ==================== INSERT su (player=3, round=3) ==================== */
    divline("PLAY insert (p=3, r=3)");
    // Elimino l'eventuale riga esistente per evitare vincoli di PK duplicata
    // (void)delete_play_by_pk(db, 3, 3);

    Play new_p;
    new_p.id_player = 3;  // esistono (1..3)
    new_p.id_round  = 3;  // esistono (1..3)
    new_p.result    = WIN;

    pst = insert_play(db, &new_p);
    printf("[insert p=3 r=3] -> %s\n", return_play_status_to_string(pst));
    if (pst != PLAY_OK) { db_close(db); return 1; }

    Play fetched;
    pst = get_play_by_pk(db, 3, 3, &fetched);
    printf("[get_by_pk p=3 r=3 dopo insert] -> %s\n", return_play_status_to_string(pst));
    if (pst == PLAY_OK) print_play(&fetched, "  ");

    /* ==================== UPDATE sulla riga inserita ==================== */
    divline("PLAY update (sulla riga inserita p=3 r=3)");
    Play before = fetched;

    // Cambia il result in modo certo (diverso dall'attuale)
    Play to_update = before;
    to_update.result = (before.result == WIN ? LOSE : WIN);

    pst = update_play_by_pk(db, &to_update);
    printf("[update p=3 r=3] -> %s\n", return_play_status_to_string(pst));

    Play after;
    pst = get_play_by_pk(db, 3, 3, &after);
    printf("[get_by_pk p=3 r=3 dopo update] -> %s\n", return_play_status_to_string(pst));
    if (pst == PLAY_OK) print_play(&after, "  ");

    /* Provo un update NOT_MODIFIED (stesso result) */
    divline("PLAY update (NOT_MODIFIED atteso)");
    pst = update_play_by_pk(db, &after);
    printf("[update identico p=3 r=3] -> %s (atteso: NOT_MODIFIED)\n",
           return_play_status_to_string(pst));

    /* ==================== DELETE (pulizia) ==================== */
    // divline("PLAY delete (pulizia p=3 r=3)");
    // pst = delete_play_by_pk(db, 3, 3);
    // printf("[delete p=3 r=3] -> %s\n", return_play_status_to_string(pst));

    Play probe;
    pst = get_play_by_pk(db, 3, 3, &probe);
    printf("[get_by_pk p=3 r=3 dopo delete] -> %s (atteso: NOT_FOUND)\n",
           return_play_status_to_string(pst));

    /* ==================== TEST INPUT INVALIDI (rapidi) ==================== */
    divline("PLAY invalid inputs (smoke tests)");
    pst = get_play_by_pk(db, 0, 1, &probe);
    printf("[get_by_pk p=0 r=1] -> %s (atteso: INVALID_INPUT)\n",
           return_play_status_to_string(pst));

    pst = delete_play_by_pk(db, 1, 0);
    printf("[delete p=1 r=0] -> %s (atteso: INVALID_INPUT)\n",
           return_play_status_to_string(pst));

    Play bad;
    bad.id_player = 1;
    bad.id_round  = 1;
    bad.result    = PLAY_RESULT_INVALID; // fuori range
    pst = insert_play(db, &bad);
    printf("[insert result invalid] -> %s (atteso: INVALID_INPUT)\n",
           return_play_status_to_string(pst));

    db_close(db);
    return 0;
}