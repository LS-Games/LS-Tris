#ifndef PARTICIPATION_REQUEST_ENTITY_H
#define PARTICIPATION_REQUEST_ENTITY_H

#define DATE_MAX 100

#include <stdint.h>
#include <time.h>

typedef enum {
    PENDING, 
    ACCEPTED,
    REJECTED,
    REQUEST_STATUS_INVALID
} RequestStatus;

typedef struct ParticipationRequest {
    int64_t id_request;
    int64_t id_player;
    int64_t id_game;
    time_t created_at;
    RequestStatus state;
} ParticipationRequest;

void print_participation_request(const ParticipationRequest *pr);
void print_participation_request_inline(const ParticipationRequest *pr);

const char* request_participation_status_to_string(RequestStatus request);
RequestStatus string_to_request_participation_status(const char *req_str);

#endif