#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../../include/debug_log.h"

#include "notification_controller.h"
#include "game_controller.h"
#include "round_controller.h"
#include "participation_request_controller.h"

NotificationControllerStatus notification_rematch_game(int64_t id_game, int64_t id_sender, int64_t id_receiver, NotificationDTO **out_dto) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    if (id_sender != retrievedGame.id_owner) {
        return NOTIFICATION_CONTROLLER_FORBIDDEN;
    }

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = id_receiver;
    dynamicDTO->message = "You've been invited to a rematch!";
    dynamicDTO->id_game = id_game;
    dynamicDTO->id_round = -1;

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

// ===================== Controllers Helper Functions =====================

// ===================== Notification Participation Request =====================

NotificationControllerStatus notification_participation_request_cancel(int64_t id_request, int64_t id_sender, NotificationDTO **out_dto) {

    ParticipationRequest retrievedParticipationRequest;

    ParticipationRequestControllerStatus request_status = participation_request_find_one(id_request, &retrievedParticipationRequest);

    if (request_status != PARTICIPATION_REQUEST_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    if (id_sender != retrievedParticipationRequest.id_player) {
        return NOTIFICATION_CONTROLLER_FORBIDDEN;
    }

    GameWithPlayerNickname retrievedGame;

    GameControllerStatus game_status = game_find_one_with_player_info(retrievedParticipationRequest.id_game, &retrievedGame);

    if (game_status != GAME_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = retrievedGame.id_creator;
    dynamicDTO->message = "A participation request has been canceled!";
    dynamicDTO->id_request = retrievedParticipationRequest.id_request;
    dynamicDTO->id_game = -1;
    dynamicDTO->id_round = -1;

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

NotificationControllerStatus notification_participation_request_change(int64_t id_request, int64_t id_sender, int64_t id_receiver, char *status, NotificationDTO **out_dto) {

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = id_receiver;
    dynamicDTO->message = "A participation request status has been changed";
    dynamicDTO->id_request = id_request;
    dynamicDTO->id_game = -1;
    dynamicDTO->id_round = -1;
    dynamicDTO->request_status = status;

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}


// ===================== Notification Game =====================

NotificationControllerStatus notification_new_game(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    if (id_sender != retrievedGame.id_creator) {
        return NOTIFICATION_CONTROLLER_FORBIDDEN;
    }

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = -1;
    dynamicDTO->message = "A new game has been created! Submit your request to participate!";
    dynamicDTO->id_game = id_game;
    dynamicDTO->id_round = -1;
    dynamicDTO->id_request = -1;

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

NotificationControllerStatus notification_game_cancel(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    if (id_sender != retrievedGame.id_creator) {
        return NOTIFICATION_CONTROLLER_FORBIDDEN;
    }

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender   = -1;
    dynamicDTO->id_playerReceiver = -1;
    dynamicDTO->id_game           = -1;
    dynamicDTO->id_round          = -1;
    dynamicDTO->id_request        = -1;

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_game = id_game; 
    dynamicDTO->message = "A game has been canceled!";

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

NotificationControllerStatus notification_waiting_game(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    if (id_sender != retrievedGame.id_creator) {
        return NOTIFICATION_CONTROLLER_FORBIDDEN;
    }

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = -1;
    dynamicDTO->message = "The owner of a game is waiting for players! Send your request to participate!";
    dynamicDTO->id_game = id_game;
    dynamicDTO->id_round = -1;

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

NotificationControllerStatus notification_finished_round(int64_t id_round, int64_t id_sender, const char *result, NotificationDTO **out_dto) {

    Round retrievedRound;
    RoundControllerStatus status = round_find_one(id_round, &retrievedRound);
    if (status != ROUND_CONTROLLER_OK)
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;

    NotificationDTO *dynamicDTO = malloc(sizeof(NotificationDTO));

    if (dynamicDTO == NULL) {
        LOG_WARN("%s\n", "Memory not allocated");
        return NOTIFICATION_CONTROLLER_INTERNAL_ERROR;
    }

    dynamicDTO->id_playerSender = id_sender;
    dynamicDTO->id_playerReceiver = -1;
    dynamicDTO->id_game = retrievedRound.id_game;
    dynamicDTO->id_round = id_round;

    PlayResult play_res = string_to_play_result(result);

    if (play_res == DRAW)
        dynamicDTO->message = "The round ended in a draw!";
    else if (play_res == WIN)
        dynamicDTO->message = "The round ended in victory!";

    *out_dto = dynamicDTO;

    return NOTIFICATION_CONTROLLER_OK;
}

// ===================== CRUD Operations =====================

// Funzione di utilità per messaggi di errore
const char *return_notification_controller_status_to_string(NotificationControllerStatus status) {
    switch (status) {
        case NOTIFICATION_CONTROLLER_OK:                    return "NOTIFICATION_CONTROLLER_OK";
        // case NOTIFICATION_CONTROLLER_INVALID_INPUT:      return "NOTIFICATION_CONTROLLER_INVALID_INPUT";
        // case NOTIFICATION_CONTROLLER_NOT_FOUND:          return "NOTIFICATION_CONTROLLER_NOT_FOUND";
        // case NOTIFICATION_CONTROLLER_STATE_VIOLATION:    return "NOTIFICATION_CONTROLLER_STATE_VIOLATION";
        // case NOTIFICATION_CONTROLLER_DATABASE_ERROR:     return "NOTIFICATION_CONTROLLER_DATABASE_ERROR";
        // case NOTIFICATION_CONTROLLER_CONFLICT:           return "NOTIFICATION_CONTROLLER_CONFLICT";
        case NOTIFICATION_CONTROLLER_FORBIDDEN:          return "NOTIFICATION_CONTROLLER_FORBIDDEN";
        case NOTIFICATION_CONTROLLER_INTERNAL_ERROR:        return "NOTIFICATION_CONTROLLER_INTERNAL_ERROR";
        default:                                return "NOTIFICATION_CONTROLLER_UNKNOWN";
    }
}