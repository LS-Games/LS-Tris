#include <stdio.h>
#include "./models/player_model.h"
#include "./db/db_connection.h"
#include <string.h>

int main() {

    const char *db_path = "data/database.sqlite";
    sqlite3 *db = db_open(db_path);

        // Player player;
        // PlayerStatus st = get_player_by_id(db, 2, &player);

        // if(st == PLAYER_OK) { 
        //     printf("\nIl giocatore ha id: %d\n", player.id_player);
        //     printf("\nIl giocatore ha nickname: %s\n", player.nickname);
        //     printf("\nIl giocatore ha email: %s\n", player.email);
        //     printf("\nIl giocatore ha data di registrazione: %s\n", player.registration_date);
        //     printf("\nIl giocatore ha password: %s\n", player.password);

        // } else {
        //     printf("Abbiamo un errore.");
        // }

        // Player *player;
        // int count;
        // PlayerStatus st = get_all_players(db, &player, &count);

        // if(st == PLAYER_OK) {
        //     printf("Le info del giocatore 1 sono: %s \t %s \t %d \t %s\n", player[0].nickname, player[0].email, player[0].max_streak, player[0].registration_date);
        //     printf("Le info del giocatore 2 sono: %s \t %s \t %d \t %s\n", player[1].nickname, player[1].email, player[1].max_streak, player[1].registration_date);
        //     printf("Le info del giocatore 3 sono: %s \t %s \t %d \t %s\n", player[2].nickname, player[2].email, player[2].max_streak, player[2].registration_date);

        //     printf("Il conteggio delle righe trovate è: %d\n", count);
        // } 

        // Player upd_player;
        // PlayerStatus st_upd_player = get_player_by_id(db, 2, &upd_player);

        // strcpy(upd_player.nickname, "Luca");
        // strcpy(upd_player.email, "luca@luca.com");
        // upd_player.max_streak = 10;

        // PlayerStatus st = update_player_by_id(db, 2, &upd_player);

        // if (st == PLAYER_OK) {
        //     Player rec_player;
        //     PlayerStatus player_stat = get_player_by_id(db, 2, &rec_player);
        //     if(player_stat == PLAYER_OK) {
        //         printf("\nIl nickname aggiornato è: %s ", rec_player.nickname);
        //         printf("\nL'email aggiornata è: %s ", rec_player.email);
        //         printf("\nIl max_strak aggiornato è %d", rec_player.max_streak);
        //     }
        // }

        // PlayerStatus del_ret = delete_player_by_id(db, 2);
        // printf("Stato: %s\n", player_status_to_string(del_ret));

        Player player;
        strcpy(player.nickname, "Marco" );
        strcpy(player.email, "marco@marco.com");
        strcpy(player.password, "Marco123");
        player.current_streak = 3;
        player.max_streak = 4;
        strcpy(player.registration_date, "16-08-2025");
        

        PlayerStatus res_ins_player = insert_player(db, &player);


    db_close(db);
    return 0;
}