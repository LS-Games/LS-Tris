#include <stdio.h>
#include <string.h>

#include "round_entity.h"

const char* round_status_to_string(RoundStatus state) {
    switch (state) {
        case ACTIVE_ROUND :               return "active";
        case PENDING_ROUND :              return "pending";
        case FINISHED_ROUND :             return "finished";
        default:                          return NULL;
    }
}

RoundStatus string_to_round_status(const char *state_str) {
    if (state_str) {
        if (strcmp(state_str, "active") == 0)       return ACTIVE_ROUND;
        if (strcmp(state_str, "pending") == 0)      return PENDING_ROUND;
        if (strcmp(state_str, "finished") == 0)     return FINISHED_ROUND;
    }

    return ROUND_STATUS_INVALID;
}