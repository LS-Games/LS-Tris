#include <stdio.h>
#include "./models/player_model.h"
#include "./models/game_model.h"
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
        // PlayerReturnStatus st_upd_player = get_player_by_id(db, 2, &upd_player);

        // strcpy(upd_player.nickname, "Luchett");
        // strcpy(upd_player.email, "luca@luca.com");
        // upd_player.max_streak = 10;

        // PlayerReturnStatus st = update_player_by_id(db, &upd_player);

        // if (st == PLAYER_OK) {
        //     Player rec_player;
        //     PlayerReturnStatus player_stat = get_player_by_id(db, 2, &rec_player);
        //     if(player_stat == PLAYER_OK) {
        //         printf("\nIl nickname aggiornato è: %s ", rec_player.nickname);
        //         printf("\nL'email aggiornata è: %s ", rec_player.email);
        //         printf("\nIl max_strak aggiornato è %d", rec_player.max_streak);
        //     }
        // }

        // PlayerStatus del_ret = delete_player_by_id(db, 2);
        // printf("Stato: %s\n", player_status_to_string(del_ret));

        // Player player;
        // strcpy(player.nickname, "Marco" );
        // strcpy(player.email, "marco@marco.com");
        // strcpy(player.password, "Marco123");
        // player.current_streak = 3;
        // player.max_streak = 4;
        // strcpy(player.registration_date, "16-08-2025");
        

        // Game game;
        // GameReturnStatus st = get_game_by_id(db, 3, &game);

        // if(st == GAME_OK) { 
        //     printf("\n La partita ha id_game: %d\n", game.id_game);
        //     printf("\n La partita ha id_creator: %d\n", game.id_creator);
        //     printf("\nLa partita ha id_owner: %d\n", game.id_owner);
        //     printf("\nLa partita è nello stato: %s\n", game_status_to_string(game.state));
        //     printf("\nLa partita è stata creata il: %s\n", game.created_at);

        // } else {
        //     printf("Abbiamo un errore.");
        // }



        // Game *game;
        // int count;
        // GameReturnStatus st = get_all_games(db, &game, &count);

        // if(st == GAME_OK) {
        //     printf("\nLe info della partita 1 sono: %d \t %d \t %d \t %s\n", game[0].id_game, game[0].id_creator, game[0].id_owner, game_status_to_string(game[0].state));
        //     printf("\nLe info della partita 2 sono: %d \t %d \t %d \t %s\n", game[1].id_game, game[1].id_creator, game[1].id_owner, game_status_to_string(game[1].state));
        //     printf("\nLe info della partita 3 sono: %d \t %d \t %d \t %s\n", game[2].id_game, game[2].id_creator, game[2].id_owner, game_status_to_string(game[2].state));

        //     printf("\nIl conteggio delle righe trovate è: %d\n", count);
        // } else {
        //     printf("\nc'è un errore\n");
        // }

        // Game upd_game;
        // GameReturnStatus st_upd_game = get_game_by_id(db, 2, &upd_game);

        // upd_game.id_creator = 4;
        // upd_game.id_owner = 4;
        // upd_game.state = NEW;

        // GameReturnStatus st = update_game_by_id(db, &upd_game);

        // if (st == GAME_OK) {
        //     Game rec_game;
        //     GameReturnStatus game_stat = get_game_by_id(db, 2, &rec_game);
        //     if(game_stat == PLAYER_OK) {
        //         printf("\nL'id_creator aggiornato è: %d \n", rec_game.id_creator);
        //         printf("\nL'id owner aggiornato è: %d \n", rec_game.id_owner);
        //         printf("\nLo stato aggiornato è: %s \n", game_status_to_string(rec_game.state));
        //     }
        // }

        
        // GameReturnStatus del_ret = delete_game_by_id(db, 4);
        // printf("Stato: %s\n", return_game_status_to_string(del_ret));

        // Game game;
        // game.id_owner = 4;
        // game.id_creator = 0;
        // game.state = NEW;
        // strcpy(game.created_at, "17-08-2025 20:05:00");

        // GameReturnStatus g_st = insert_game(db, (const*) &game);
        // printf("\n%s\n", return_game_status_to_string(g_st));








    db_close(db);
    return 0;
}