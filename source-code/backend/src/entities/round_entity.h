#ifndef ROUND_ENTITY_H
#define ROUND_ENTITY_H

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

const char* round_status_to_string(RoundStatus state);
RoundStatus string_to_round_status(const char *state_str);

#endif