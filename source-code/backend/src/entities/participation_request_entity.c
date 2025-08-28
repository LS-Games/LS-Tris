#include <stdio.h>
#include <string.h>

#include "participation_request_entity.h"

void print_participation_request(const ParticipationRequest *pr) {
    if (!pr) {
        printf("ParticipationRequest: (NULL)\n");
        return;
    }

    printf("ParticipationRequest {\n");
    printf("  id_request: %d\n", pr->id_request);
    printf("  id_player: %d\n", pr->id_player);
    printf("  id_game: %d\n", pr->id_game);
    printf("  created_at: \"%s\"\n", pr->created_at);
    printf("  state: \"%s\"\n", request_participation_status_to_string(pr->state));
    printf("}\n");
}

void print_participation_request_inline(const ParticipationRequest *pr) {
    if (!pr) {
        printf("ParticipationRequest(NULL)\n");
        return;
    }
    printf("ParticipationRequest[id=%d, player=%d, game=%d, created_at=%s, state=%s]\n",
            pr->id_request,
            pr->id_player,
            pr->id_game,
            pr->created_at,
            request_participation_status_to_string(pr->state));
}

const char* request_participation_status_to_string(RequestStatus state) {
    switch (state) {
        case PENDING :          return "pending";
        case ACCEPTED :         return "accepted";
        case REJECTED :         return "rejected";
        default:                return NULL;
    }
}

RequestStatus string_to_request_participation_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "pending") == 0)      return PENDING;
        if (strcmp(state_str, "accepted") == 0)     return ACCEPTED;
        if (strcmp(state_str, "rejected") == 0)     return REJECTED;
    }

    return REQUEST_STATUS_INVALID; 
}