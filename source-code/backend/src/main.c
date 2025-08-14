#include <stdio.h>
#include "./models/player_model.h"
#include "./db/db_connection.h"

int main(void) {

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

        Player *player;
        int count;
        PlayerStatus st = get_all_players(db, &player, &count);

        if(st == PLAYER_OK) {
            printf("Le info del giocatore 1 sono: %s \t %s \t %d \t %s\n", player[0].nickname, player[0].email, player[0].max_streak, player[0].registration_date);
            printf("Le info del giocatore 2 sono: %s \t %s \t %d \t %s\n", player[1].nickname, player[1].email, player[1].max_streak, player[1].registration_date);
            printf("Le info del giocatore 3 sono: %s \t %s \t %d \t %s\n", player[2].nickname, player[2].email, player[2].max_streak, player[2].registration_date);

            printf("Il conteggio delle righe trovate è: %d\n", count);
        } 

    db_close(db);
    return 0;
}