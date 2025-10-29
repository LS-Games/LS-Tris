#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "participation_request_entity.h"

void print_participation_request(const ParticipationRequest *pr) {
    if (!pr) {
        printf("ParticipationRequest: (NULL)\n");
        return;
    }

    // Compute time in UCT
    struct tm tm_utc;
    char buffer[26];
    gmtime_r(&pr->created_at, &tm_utc);
    asctime_r(&tm_utc, buffer);

    printf("ParticipationRequest {\n");
    printf("  id_request: %" PRId64 "\n", pr->id_request);
    printf("  id_player: %" PRId64 "\n", pr->id_player);
    printf("  id_game: %" PRId64 "\n", pr->id_game);
    printf("  created_at: \"%s\"\n", buffer);
    printf("  state: \"%s\"\n", request_participation_status_to_string(pr->state));
    printf("}\n");
}

void print_participation_request_inline(const ParticipationRequest *pr) {
    if (!pr) {
        printf("ParticipationRequest(NULL)\n");
        return;
    }

    // Compute time in UCT
    struct tm tm_utc;
    char buffer[26];
    gmtime_r(&pr->created_at, &tm_utc);
    asctime_r(&tm_utc, buffer);

    printf("ParticipationRequest[id=%" PRId64 ", player=%" PRId64 ", game=%" PRId64 ", created_at=%s, state=%s]\n",
            pr->id_request,
            pr->id_player,
            pr->id_game,
            buffer,
            request_participation_status_to_string(pr->state));
}

const char *request_participation_status_to_string(RequestStatus state) {
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