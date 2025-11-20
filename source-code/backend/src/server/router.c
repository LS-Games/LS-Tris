#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../../include/debug_log.h"

#include "server.h"
#include "session_manager.h"
#include "../json-parser/json-parser.h"

#include "../dto/game_dto.h"
#include "../dto/notification_dto.h"
#include "../dto/participation_request_dto.h"
#include "../dto/play_dto.h"
#include "../dto/player_dto.h"
#include "../dto/round_dto.h"

#include "../controllers/game_controller.h"
#include "../controllers/notification_controller.h"
#include "../controllers/participation_request_controller.h"
#include "../controllers/play_controller.h"
#include "../controllers/player_controller.h"
#include "../controllers/round_controller.h"

void route_request(const char* json_body, int client_socket, int* persistence) {

    LOG_DEBUG("Received JSON: '%s'\n", json_body);

    char *action = extract_string_from_json(json_body, "action");

    *persistence = 1; 

    if (!action) {
        LOG_WARN("%s\n", "Missing 'action' key in JSON");
        action = "NULL";
    }

    /* === Extracted value === */

    int64_t id_player = extract_int_from_json(json_body, "id_player");
    int64_t id_game = extract_int_from_json(json_body, "id_game");
    int64_t id_round = extract_int_from_json(json_body, "id_round");
    int64_t id_participation_request = extract_int_from_json(json_body, "id_participation_request");
    
    // Player controller input
    char *nickname = extract_string_from_json(json_body, "nickname");
    char *email = extract_string_from_json(json_body, "email");
    char *password = extract_string_from_json(json_body, "password");

    // Game controller input
    char *status = extract_string_from_json(json_body, "status");
    int64_t id_creator = extract_int_from_json(json_body, "id_creator");
    int64_t id_owner = extract_int_from_json(json_body, "id_owner");
    int64_t id_player_accepting_rematch = extract_int_from_json(json_body, "id_player_accepting_rematch");

    // Round controller input
    int row = extract_int_from_json(json_body, "row");
    int col = extract_int_from_json(json_body, "col");
    int64_t id_player_ending_round = extract_int_from_json(json_body, "id_player_ending_round");

    // Participation Request controller input
    char *state = extract_string_from_json(json_body, "state");
    char *new_state = extract_string_from_json(json_body, "new_state");

    // Notification controller input
    int64_t id_sender = extract_int_from_json(json_body, "id_sender");
    int64_t id_receiver = extract_int_from_json(json_body, "id_receiver");


    /* === Result value === */

    int out_count = 0;

    // Player controller output
    bool out_signedIn = false;
    int64_t out_id_player = -1;
    PlayerDTO *out_player = NULL;
    
    // Game controller output
    int64_t out_id_game = -1;
    GameDTO *out_games = NULL;

    // Round controller output
    int64_t out_id_round = -1;
    RoundDTO *out_round = NULL;

    // Participation Request controller output
    int64_t out_id_participation_request = -1;
    ParticipationRequestDTO *out_participation_requests = NULL;

    // Play controller output
    PlayDTO *out_plays = NULL;

    // Notification controller output
    NotificationDTO *out_notification = NULL;


    /* === Router === */

    char *json_response = NULL;

    if (strcmp(action, "NULL") == 0) {
        json_response = serialize_action_error(action, "Missing 'action' key");
    } else

    // Player routes
    if (strcmp(action, "player_get_public_info") == 0) {
        PlayerControllerStatus playerStatus = player_get_public_info(nickname, &out_player, &out_count);
        if (playerStatus == PLAYER_CONTROLLER_OK || playerStatus == PLAYER_CONTROLLER_NOT_FOUND) {
            json_response = serialize_players_to_json(action, out_player, out_count);
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }

    } else if (strcmp(action, "player_signup") == 0) {
        PlayerControllerStatus playerStatus = player_signup(nickname, email, password);
        if (playerStatus == PLAYER_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Player signed up", -1);
        } else if (playerStatus == PLAYER_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else if (playerStatus == PLAYER_CONTROLLER_STATE_VIOLATION_NICKNAME) {
            json_response = serialize_action_error(action, "Nickname already used");
        } else if (playerStatus == PLAYER_CONTROLLER_STATE_VIOLATION_EMAIL) {
            json_response = serialize_action_error(action, "Email already used");
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }

        *persistence = 0;

    } else if (strcmp(action, "player_signin") == 0) {
        PlayerControllerStatus playerStatus = player_signin(nickname, password, &out_signedIn, &out_id_player);
        if (playerStatus == PLAYER_CONTROLLER_OK) {
            if (out_signedIn == true) {
                json_response = serialize_action_success(action, "Player signed in", out_id_player);

                // We add a session if the user logged in succesfully
                session_add(&session_manager, client_socket, out_id_player, nickname);
                LOG_INFO("Player \"%s\" (id_player %" PRId64 ") has been added in session list", nickname, out_id_player);

            } else
                json_response = serialize_action_error(action, "Log in failed");
        } else if (playerStatus == PLAYER_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }

    } else 

    // Game routes
    if (strcmp(action, "games_get_public_info") == 0) {
        GameControllerStatus gameStatus = games_get_public_info(status, &out_games, &out_count);
        if (gameStatus == GAME_CONTROLLER_OK || gameStatus == GAME_CONTROLLER_NOT_FOUND) {
            json_response = serialize_games_to_json(action, out_games, out_count);
        } else if (gameStatus == GAME_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else if (strcmp(action, "game_start") == 0) {
        GameControllerStatus gameStatus = game_start(id_creator, &out_id_game);
        if (gameStatus == GAME_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Game started", out_id_game);
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else if (strcmp(action, "game_end") == 0) { // Sent by the game owner
        GameControllerStatus gameStatus = game_end(id_game, id_owner, &out_id_game);
        if (gameStatus == GAME_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Game closed", out_id_game);
        } else if (gameStatus == GAME_CONTROLLER_FORBIDDEN) {
            json_response = serialize_action_error(action, "Action not allowed");
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else if (strcmp(action, "game_refuse_rematch") == 0) { // Sent by the player who got the rematch notification
        GameControllerStatus gameStatus = game_refuse_rematch(id_game, &out_id_game);
        if (gameStatus == GAME_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Rematch refused", out_id_game);
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else if (strcmp(action, "game_accept_rematch") == 0) { // Sent by the player who got the rematch notification
        GameControllerStatus gameStatus = game_accept_rematch(id_game, id_player_accepting_rematch, &out_id_game);
        if (gameStatus == GAME_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Rematch accepted", out_id_game);
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else if (strcmp(action, "game_cancel") == 0) { // Sent by the game creator, accepted if the game has no round finished
        LOG_DEBUG("ID_GAME: %" PRId64 ", ID_OWNER: %" PRId64 "\n", id_game, id_owner);
        GameControllerStatus gameStatus = game_cancel(id_game, id_owner, &out_id_game);
        if (gameStatus == GAME_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Game canceled", out_id_game);
        } else {
            json_response = serialize_action_error(action, return_game_controller_status_to_string(gameStatus));
        }

    } else 

    // Round routes
    if (strcmp(action, "round_get_public_info") == 0) {
        RoundControllerStatus roundStatus = round_get_public_info(id_round, &out_round, &out_count);
        if (roundStatus == ROUND_CONTROLLER_OK || roundStatus == ROUND_CONTROLLER_NOT_FOUND) {
            json_response = serialize_rounds_to_json(action, out_round, out_count);
        } else {
            json_response = serialize_action_error(action, return_round_controller_status_to_string(roundStatus));
        }

    } else if (strcmp(action, "round_make_move") == 0) { // Sent by one of the players in the round
        RoundControllerStatus roundStatus = round_make_move(id_round, id_player, row, col, &out_id_round);
        if (roundStatus == ROUND_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Move registered", out_id_round);
        } else if (roundStatus == ROUND_CONTROLLER_STATE_VIOLATION) {
            json_response = serialize_action_error(action, "Not an active round");
        } else if (roundStatus == ROUND_CONTROLLER_FORBIDDEN) {
            json_response = serialize_action_error(action, "Action not allowed");
        } else if (roundStatus == ROUND_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_round_controller_status_to_string(roundStatus));
        }

    } else if (strcmp(action, "round_end") == 0) { // Sent by one of the players in the round
        RoundControllerStatus roundStatus = round_end(id_round, id_player_ending_round, &out_id_round);
        if (roundStatus == ROUND_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Round closed", out_id_round);
        } else {
            json_response = serialize_action_error(action, return_round_controller_status_to_string(roundStatus));
        }

    } else

    // Participation Request routes
    if (strcmp(action, "participation_requests_get_public_info") == 0) {
        ParticipationRequestControllerStatus participationRequestStatus = participation_requests_get_public_info(state, id_game, &out_participation_requests, &out_count);
        if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_OK || participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_NOT_FOUND) {
            json_response = serialize_participation_requests_to_json(action, out_participation_requests, out_count);
        } else if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_participation_request_controller_status_to_string(participationRequestStatus));
        }

    } else if (strcmp(action, "participation_request_send") == 0) {
        ParticipationRequestControllerStatus participationRequestStatus = participation_request_send(id_game, id_player, &out_id_participation_request);
        if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Participation request sent", out_id_participation_request);
        } else {
            json_response = serialize_action_error(action, return_participation_request_controller_status_to_string(participationRequestStatus));
        }

    } else if (strcmp(action, "participation_request_change_state") == 0) { // Sent by the game owner
        ParticipationRequestControllerStatus participationRequestStatus = participation_request_change_state(id_participation_request, new_state, &out_id_participation_request);
        if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Participation request state changed", out_id_participation_request);
        } else if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_participation_request_controller_status_to_string(participationRequestStatus));
        }

    } else if (strcmp(action, "participation_request_cancel") == 0) { // Sent by the participation request sender
        ParticipationRequestControllerStatus participationRequestStatus = participation_request_cancel(id_participation_request, id_player, &out_id_participation_request);
        if (participationRequestStatus == PARTICIPATION_REQUEST_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Participation request canceled", out_id_participation_request);
        } else {
            json_response = serialize_action_error(action, return_participation_request_controller_status_to_string(participationRequestStatus));
        }

    } else

    // Play routes
    if (strcmp(action, "plays_get_public_info") == 0) {
        PlayControllerStatus playStatus = plays_get_public_info(id_player, id_round, &out_plays, &out_count);
        if (playStatus == PLAY_CONTROLLER_OK || playStatus == PLAY_CONTROLLER_NOT_FOUND) {
            json_response = serialize_plays_to_json(action, out_plays, out_count);
        } else {
            json_response = serialize_action_error(action, return_play_controller_status_to_string(playStatus));
        }

    } else
    
    // Notification routes
    if (strcmp(action, "notification_rematch_game") == 0) { // Sent by the game owner
        NotificationControllerStatus notificationStatus = notification_rematch_game(id_game, id_sender, id_receiver, &out_notification);
        if (notificationStatus == NOTIFICATION_CONTROLLER_OK) {
            char *json_message = serialize_notification_to_json(NULL, out_notification);
            if (send_server_unicast_message(json_message, id_receiver) < 0 ) {
                json_response = serialize_action_error(action, "Could not send rematch invitation");
            } else {
                json_response = serialize_action_success(action, "Rematch invitation sent", -1);
            }
            free(json_message);
        } else {
            json_response = serialize_action_error(action, return_notification_controller_status_to_string(notificationStatus));
        }

    } else {
        json_response = serialize_action_error(action, "Action not recognized");
    }


    /* === Send response === */

    LOG_INFO("Server Router Response successfully built: %s\n", json_response);
    if (json_response) {
        if (send_server_response(client_socket, json_response) < 0) {
            LOG_WARN("Error sending the Server Router Response to Client socket %d\n", client_socket);
        } else {
            LOG_DEBUG("Server Router Response sent: Client socket %d - Response: %s\n", client_socket, json_response);
        }
    } else {
        LOG_WARN("%s\n", "JSON response is empty");
        return;
    }
    

    /* === Free dynamically allocated variables */

    if (action && (strcmp(action, "NULL") != 0))
        free(action); 

    if (nickname)
        free(nickname);
    if (email)
        free(email);
    if (password)
        free(password);
    if (out_player)
        free(out_player);
    
    if (status)
        free(status);
    if (out_games)
        free(out_games);

    if (out_round)
        free(out_round);

    if (state)
        free(state);
    if (new_state)
        free(new_state);
    if (out_participation_requests)
        free(out_participation_requests);

    if (out_plays)
        free(out_plays);

    if (out_notification)
        free(out_notification);

    if (json_response)
        free(json_response);
}