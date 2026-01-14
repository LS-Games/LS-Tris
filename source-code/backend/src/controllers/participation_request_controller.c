#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/debug_log.h"

#include "participation_request_controller.h"
#include "round_controller.h"
#include "game_controller.h"
#include "player_controller.h"
#include "notification_controller.h"
#include "../json-parser/json-parser.h"
#include "../server/server.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/participation_request_dao_sqlite.h"

// ==================== Private functions ====================

static ParticipationRequestControllerStatus participation_request_accept_helper(ParticipationRequest *participation_request, RoundFullDTO *out_fullRound);

// ===========================================================

// This function provides a query by `state` and `id_game`. 
// @param state Possible values are `pending`, `accepted`, `rejected` and `all` (no filter)
// @param id_game Possible values are all integer positive number and -1 (no filter)
ParticipationRequestControllerStatus participation_requests_get_public_info(char *state, int64_t id_game, ParticipationRequestDTO **out_dtos, int *out_count) {

    RequestStatus queryState = REQUEST_STATUS_INVALID;
    if (strcmp(state, "all") != 0) {
        queryState = string_to_request_participation_status(state);
        if (queryState == REQUEST_STATUS_INVALID)
            return PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT;
    }

    ParticipationRequestWithPlayerNickname* retrievedParticipationRequestsWithPlayerNickname;
    int retrievedObjectCount;
    if (participation_request_find_all_with_player_info(&retrievedParticipationRequestsWithPlayerNickname, &retrievedObjectCount) == PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND) {
        *out_dtos = NULL;
        *out_count = 0;
        return PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND;
    }

    ParticipationRequestDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if ((strcmp(state, "all") == 0 || retrievedParticipationRequestsWithPlayerNickname[i].state == queryState) &&
            (id_game == -1 || retrievedParticipationRequestsWithPlayerNickname[i].id_game == id_game)) {

            ParticipationRequest participationRequest = {
                .id_request = retrievedParticipationRequestsWithPlayerNickname[i].id_request,
                .id_game = retrievedParticipationRequestsWithPlayerNickname[i].id_game,
                .created_at = retrievedParticipationRequestsWithPlayerNickname[i].created_at,
                .state = retrievedParticipationRequestsWithPlayerNickname[i].state
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(ParticipationRequestDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
            }

            map_participation_request_to_dto(&participationRequest, retrievedParticipationRequestsWithPlayerNickname[i].player_nickname, &(dynamicDTOs[filteredObjectCount]));
        
            filteredObjectCount = filteredObjectCount + 1;
        }
    }

    *out_dtos = dynamicDTOs;
    *out_count = filteredObjectCount;
    
    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

ParticipationRequestControllerStatus participation_request_send(int64_t id_game, int64_t id_sender, int64_t* out_id_participation_request) {

    // Build participation request to send
    ParticipationRequest participationRequestToSend = {
        .id_game = id_game,
        .id_player = id_sender,
        .created_at = time(NULL),
        .state = PENDING
    };

    ParticipationRequestControllerStatus status = participation_request_create(&participationRequestToSend);
    if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return status;

    Game retrievedGame; // Retrieve game owner
    if (game_find_one(participationRequestToSend.id_game, &retrievedGame) != GAME_CONTROLLER_OK) {
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }
    Player retrievedPlayer; // Retrieve sender nickname
    if (player_find_one(participationRequestToSend.id_player, &retrievedPlayer) != PLAYER_CONTROLLER_OK) {
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }
    ParticipationRequestDTO out_participation_request_dto; // Build message
    map_participation_request_to_dto(&participationRequestToSend, retrievedPlayer.nickname, &out_participation_request_dto);
    char *json_message = serialize_participation_requests_to_json("server_new_participation_request", &out_participation_request_dto, 1);
    if (send_server_unicast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }

    /**
     * Create participation request only if the session exists
     * Because send_server_unicast_message returns an error if the session doesn't exists
     */

    free(json_message);
    
    *out_id_participation_request = participationRequestToSend.id_request;

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

ParticipationRequestControllerStatus participation_request_change_state(int64_t id_participation_request, char *newState, int64_t* out_id_participation_request) {

    ParticipationRequest req;
    ParticipationRequestControllerStatus status = participation_request_find_one(id_participation_request, &req);
    if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return status;

    RequestStatus state = string_to_request_participation_status(newState);
    if (state == REQUEST_STATUS_INVALID)
        return PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT;

    req.state = state;

    // Retrieve game BEFORE doing anything else
    Game game;
    if (game_find_one(req.id_game, &game) != GAME_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    if (req.state == ACCEPTED) {

        RoundFullDTO fullRound;
        memset(&fullRound, 0, sizeof(RoundFullDTO));

        status = participation_request_accept_helper(&req, &fullRound);

        if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return status;

        if (participation_request_update(&req) != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

        NotificationDTO *notif = NULL;

        if (notification_participation_request_change(
                id_participation_request,
                game.id_owner,
                req.id_player,
                newState,
                &notif) != NOTIFICATION_CONTROLLER_OK)
        {
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }

        char *notif_json = serialize_notification_to_json("server_participation_request_change", notif);
        send_server_unicast_message(notif_json, notif->id_playerReceiver);
        free(notif_json);
        free(notif);

        char *json_owner = serialize_round_full_to_json("server_round_start", &fullRound);
        char *json_player = serialize_round_full_to_json("server_round_start", &fullRound);

        if (!json_owner || !json_player) {
            free(json_owner);
            free(json_player);
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }

        // Owner
        if (send_server_unicast_message(json_owner, game.id_owner) < 0) {
            free(json_owner);
            free(json_player);
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }

        LOG_INFO("ID OWNER: %d", game.id_owner);

        // Accepted player
        if (send_server_unicast_message(json_player, req.id_player) < 0) {
            free(json_owner);
            free(json_player);
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }

        LOG_INFO("ID OWNER: %d", req.id_player);

        free(json_owner);
        free(json_player);

        *out_id_participation_request = req.id_request;
        return PARTICIPATION_REQUEST_CONTROLLER_OK;
    }

    if (participation_request_update(&req) != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    NotificationDTO *notif = NULL;
    if (notification_participation_request_change(
            id_participation_request,
            game.id_owner,
            req.id_player,
            newState,
            &notif) != NOTIFICATION_CONTROLLER_OK)
    {
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }

    char *json_message = serialize_notification_to_json("server_participation_request_change", notif);

    if (notif->id_playerReceiver > 0)
        send_server_unicast_message(json_message, notif->id_playerReceiver);

    free(json_message);
    free(notif);

    *out_id_participation_request = req.id_request;
    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

static ParticipationRequestControllerStatus participation_request_accept_helper(ParticipationRequest *req, RoundFullDTO *out_fullRound) {

    int64_t id_game = req->id_game;
    int64_t id_player = req->id_player;

    Game game;
    if (game_find_one(id_game, &game) != GAME_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    int64_t new_round_id;

    if (new_round_id <= 0) {
        LOG_ERROR("round_start returned invalid round id: %" PRId64, new_round_id);
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }

    if (round_start(game.id_game, game.id_owner, id_player, &new_round_id)
        != ROUND_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    // Game → ACTIVE
    game.state = ACTIVE_GAME;
    if (game_update(&game) != GAME_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    // Reject ALL pending
    ParticipationRequest *pending = NULL;
    int count = 0;

    if (participation_request_find_all_pending_by_id_game(&pending, id_game, &count)
        != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    if (participation_request_reject_all(pending, count)
        != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    if (round_find_full_info_by_id_round(new_round_id, out_fullRound)
        != ROUND_CONTROLLER_OK)
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// ParticipationRequestControllerStatus participation_request_accept(int64_t id_participation_request, int64_t id_owner) {

//     ParticipationRequest req;
//     ParticipationRequestControllerStatus status = participation_request_find_one(id_participation_request, &req);

//     if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
//         return status;

//     //Update accepted request
//     req.state = ACCEPTED;

//     status = participation_request_update(&req);
//     if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
//         return status;

//     return participation_request_accept_helper(&req);
// }


ParticipationRequestControllerStatus participation_request_reject_all(ParticipationRequest* pendingRequestsToReject, int count) {
    
    for (int i = 0; i < count; i++) {

        ParticipationRequest dbRequest;
        ParticipationRequestControllerStatus status = participation_request_find_one(pendingRequestsToReject[i].id_request, &dbRequest);

        if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return status;

        if (dbRequest.state != PENDING)
            continue;

        dbRequest.state = REJECTED;

        status = participation_request_update(&dbRequest);
        if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
            return status;

        Game retrivedGame;
        GameControllerStatus game_status = game_find_one(dbRequest.id_game, &retrivedGame);

        if (game_status != GAME_CONTROLLER_OK)
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;

        NotificationDTO *out_notification_dto = NULL;
        if (notification_participation_request_change(
                dbRequest.id_request,
                retrivedGame.id_owner,
                dbRequest.id_player,
                "rejected",
                &out_notification_dto) != NOTIFICATION_CONTROLLER_OK) {

            LOG_WARN("ERRORE IN notification_participation_request_cancel");
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }

        char *json_message = serialize_notification_to_json("server_participation_request_change", out_notification_dto);

        if (out_notification_dto->id_playerReceiver > 0) {
            if (send_server_unicast_message(json_message, out_notification_dto->id_playerReceiver) < 0) {

                free(json_message);
                free(out_notification_dto);
                return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
            }
        }

        free(json_message);
        free(out_notification_dto);
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

ParticipationRequestControllerStatus participation_request_cancel(int64_t id_participation_request, int64_t id_sender, int64_t* out_id_participation_request) {

    NotificationDTO *out_notification_dto = NULL;

    if(notification_participation_request_cancel(id_participation_request, id_sender, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK) {
        return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
    }

    char *json_message = serialize_notification_to_json("server_participation_request_cancel", out_notification_dto);
    LOG_DEBUG("%s", json_message);

    if(out_notification_dto->id_playerReceiver > 0) {
        if(send_server_unicast_message(json_message, out_notification_dto->id_playerReceiver) < 0) {
            free(json_message);
            free(out_notification_dto);
            return PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR;
        }
    }

    free(json_message);
    free(out_notification_dto);

    ParticipationRequestControllerStatus status = participation_request_delete(id_participation_request);
    if (status != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return status;

    *out_id_participation_request = id_participation_request;

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

const char *return_participation_request_controller_status_to_string(ParticipationRequestControllerStatus status) {
    switch (status) {
        case PARTICIPATION_REQUEST_CONTROLLER_OK:               return "PARTICIPATION_REQUEST_CONTROLLER_OK";
        case PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT:    return "PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT";
        case PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND:        return "PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND";
        // case PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION:  return "PARTICIPATION_REQUEST_CONTROLLER_STATE_VIOLATION";
        case PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR";
        // case PARTICIPATION_REQUEST_CONTROLLER_CONFLICT:         return "PARTICIPATION_REQUEST_CONTROLLER_CONFLICT";
        // case PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN:        return "PARTICIPATION_REQUEST_CONTROLLER_FORBIDDEN";
        case PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR:   return "PARTICIPATION_REQUEST_CONTROLLER_INTERNAL_ERROR";
        default:                                return "PARTICIPATION_REQUEST_CONTROLLER_UNKNOWN";
    }
}

// Create
ParticipationRequestControllerStatus participation_request_create(ParticipationRequest* participationRequestToCreate) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = insert_participation_request(db, participationRequestToCreate);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all
ParticipationRequestControllerStatus participation_request_find_all(ParticipationRequest **retrievedParticipationRequestArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_participation_requests(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read one
ParticipationRequestControllerStatus participation_request_find_one(int64_t id_participation_request, ParticipationRequest* retrievedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_participation_request_by_id(db, id_participation_request, retrievedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Update
ParticipationRequestControllerStatus participation_request_update(ParticipationRequest* updatedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = update_participation_request_by_id(db, updatedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Delete
ParticipationRequestControllerStatus participation_request_delete(int64_t id_participation_request) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = delete_participation_request_by_id(db, id_participation_request);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read one with player info
ParticipationRequestControllerStatus participation_request_find_one_with_player_info(int64_t id_participation_request, ParticipationRequestWithPlayerNickname* retrievedParticipationRequest) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_participation_request_by_id_with_player_info(db, id_participation_request, retrievedParticipationRequest);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return status == PARTICIPATION_DAO_REQUEST_NOT_FOUND ? PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND : PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all with player info
ParticipationRequestControllerStatus participation_request_find_all_with_player_info(ParticipationRequestWithPlayerNickname **retrievedParticipationRequestArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_participation_requests_with_player_info(db, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}

// Read all (state="pending" by id_game)
ParticipationRequestControllerStatus participation_request_find_all_pending_by_id_game(ParticipationRequest **retrievedParticipationRequestArray, int64_t id_game, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    ParticipationRequestDaoStatus status = get_all_pending_participation_request_by_id_game(db, id_game, retrievedParticipationRequestArray, retrievedObjectCount);
    db_close(db);
    if (status != PARTICIPATION_DAO_REQUEST_OK) {
        LOG_WARN("%s\n", return_participation_request_dao_status_to_string(status));
        return PARTICIPATION_REQUEST_CONTROLLER_DATABASE_ERROR;
    }

    return PARTICIPATION_REQUEST_CONTROLLER_OK;
}