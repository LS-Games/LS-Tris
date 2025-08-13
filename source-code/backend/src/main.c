#include <stdio.h>
#include "./models/player_model.h"
#include "./db/db_connection.h"

int main(void) {

    const char *db_path = "data/database.sqlite";
    sqlite3 *db = db_open(db_path);

        Player player;
        PlayerStatus st = get_player_by_id(db, 2, &player);

        if(st == PLAYER_OK) { 
            printf("\nIl giocatore ha id: %d\n", player.id_player);
            printf("\nIl giocatore ha nickname: %s\n", player.nickname);
            printf("\nIl giocatore ha email: %s\n", player.email);
            printf("\nIl giocatore ha data di registrazione: %s\n", player.registration_date);
            printf("\nIl giocatore ha password: %s\n", player.password);

        } else {
            printf("Abbiamo un errore.");
        }

    db_close(db);
    return 0;
}