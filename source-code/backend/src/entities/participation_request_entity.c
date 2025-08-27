#include <stdio.h>
#include <string.h>

#include "participation_request_entity.h"

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