#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../../include/debug_log.h"

#include "server.h"
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

void route_request(const char* json_body, int client_socket) {

    LOG_DEBUG("Received JSON: '%s'\n", json_body);

    char* action = extract_string_from_json(json_body, "action");

    if (!action) {
        LOG_WARN("%s\n", "Missing 'action' key in JSON");
        return;
    }

    /* === Extracted value === */

    // Player controller input
    char* nickname = extract_string_from_json(json_body, "nickname");
    char* email = extract_string_from_json(json_body, "email");
    char* password = extract_string_from_json(json_body, "password");

    // Notification controller input
    int64_t id_game = extract_int_from_json(json_body, "id_game");
    int64_t id_sender = extract_int_from_json(json_body, "id_sender");
    int64_t id_receiver = extract_int_from_json(json_body, "id_receiver");


    /* === Result value === */

    // Player controller output
    bool signedIn = false;
    PlayerDTO *player = NULL;

    // Notification controller output
    char* result = NULL;
    NotificationDTO *notification = NULL;


    /* === Router === */
    char *json_response = NULL;

    if (strcmp(action, "player_get_public_info") == 0) { // Player routes
        PlayerControllerStatus playerStatus = player_get_public_info(nickname, &player);
        if (playerStatus == PLAYER_CONTROLLER_OK) {
            json_response = serialize_player_to_json(player);
        } else if (playerStatus == PLAYER_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }
    } else if (strcmp(action, "player_signup") == 0) {
        PlayerControllerStatus playerStatus = player_signup(nickname, email, password);
        if (playerStatus == PLAYER_CONTROLLER_OK) {
            json_response = serialize_action_success(action, "Your account has been successfully created.");
        } else if (playerStatus == PLAYER_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else if (playerStatus == PLAYER_CONTROLLER_STATE_VIOLATION) {
            json_response = serialize_action_error(action, "Nickname already used");
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }
    } else if (strcmp(action, "player_signin") == 0) {
        PlayerControllerStatus playerStatus = player_signin(nickname, password, &signedIn);
        if (playerStatus == PLAYER_CONTROLLER_OK) {
            if (signedIn == true)
                json_response = serialize_action_success(action, NULL);
            else
                json_response = serialize_action_error(action, "Log in failed");
        } else if (playerStatus == PLAYER_CONTROLLER_INVALID_INPUT) {
            json_response = serialize_action_error(action, "Invalid input values");
        } else {
            json_response = serialize_action_error(action, return_player_controller_status_to_string(playerStatus));
        }
    } else if (strcmp(action, "rematch_game") == 0) { // Notification routes

       if (notification_rematch_game(id_game, id_sender, id_receiver, &notification) == NOTIFICATION_CONTROLLER_OK) {
            json_response = serialize_notification_to_json(notification);
       }

    } else if (strcmp(action, "new_game") == 0) {

        if (notification_new_game(id_game, id_sender, id_receiver, &notification) == NOTIFICATION_CONTROLLER_OK) {
            json_response = serialize_notification_to_json(notification);
        }
    
    } else if (strcmp(action, "waiting_game") == 0) {

        if (notification_waiting_game(id_game, id_sender, id_receiver, &notification) == NOTIFICATION_CONTROLLER_OK) {
            json_response = serialize_notification_to_json(notification);
        }
        

    } else if (strcmp(action, "finished_round") == 0) {

        int64_t id_round = extract_int_from_json(json_body, "id_round");
        result = extract_string_from_json(json_body, "result");

        if (notification_finished_round(id_round, id_sender, id_receiver, result, &notification) == NOTIFICATION_CONTROLLER_OK) {
            json_response = serialize_notification_to_json(notification);
        }
    }


    /* === Send response === */

    //DEBUG
    LOG_INFO("\n\nRISPOSTA DEL ROUTER DEL SERVER: %c", json_response);
    if (json_response) {
        server_send(client_socket, json_response);
        LOG_DEBUG("Client socket %d - Sent response: %s\n", client_socket, json_response);
    } else {
        LOG_WARN("%s\n", "JSON response is empty");
        return;
    }
    

    /* === Free dynamically allocated variables */

    if (action)             free(action); 

    if (nickname)           free(nickname);
    if (email)              free(email);
    if (password)           free(password);
    if (player)             free(player);

    if (result)             free(result);
    if (notification)       free(notification);

    if (json_response)      free(json_response);    
}