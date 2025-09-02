#ifndef PARTICIPATION_REQUEST_JOIN_PLAYER_H
#define PARTICIPATION_REQUEST_JOIN_PLAYER_H

#include "../../../entities/participation_request_entity.h"
#include "../../../entities/player_entity.h"

typedef struct {
    int64_t id_request;
    int64_t id_player;
    int64_t id_game;
    time_t created_at;
    RequestStatus state;
    char player_nickname[NICKNAME_MAX];
} ParticipationRequestWithPlayerNickname;

#endif