#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#define DATE_MAX 100

typedef enum {
    NEW_GAME,
    ACTIVE_GAME,
    WAITING_GAME,
    FINISHED_GAME,
    GAME_STATUS_INVALID
} GameStatus;

typedef struct Game {
    int id_game;
    int id_creator;
    int id_owner;
    GameStatus state;
    char created_at[DATE_MAX];
} Game;

const char* game_status_to_string(GameStatus state);
GameStatus string_to_game_status(const char *state_str);

#endif