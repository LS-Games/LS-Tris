#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#define DATE_MAX 100

#include <stdint.h>
#include <time.h>

typedef enum {
    NEW_GAME,
    ACTIVE_GAME,
    WAITING_GAME,
    FINISHED_GAME,
    GAME_STATUS_INVALID
} GameStatus;

typedef struct Game {
    int64_t id_game;
    int64_t id_creator;
    int64_t id_owner;
    GameStatus state;
    time_t created_at;
} Game;

void print_game(const Game *g);
void print_game_inline(const Game *g);

const char *game_status_to_string(GameStatus state);
GameStatus string_to_game_status(const char *state_str);

#endif