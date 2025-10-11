#include <string.h>
#include <stdlib.h>

#include "../../include/debug_log.h"

#include "server.h"
#include "../json-parser/json-parser.h"
#include "../dto/notification_dto.h"
#include "../controllers/notification_controller.h"

void route_request(const char* json_body, int client_socket) {

    char* action = extract_string_from_json(json_body, "action");

    if (!action) {
        LOG_WARN("%s\n", "Missing 'action' key in JSON");
        return;
    }

    char* result = NULL;
    int64_t id_game = extract_int_from_json(json_body, "id_game");
    int64_t id_sender = extract_int_from_json(json_body, "id_sender");
    int64_t id_receiver = extract_int_from_json(json_body, "id_receiver");

    NotificationDTO *notification = NULL;
    char *json_response = NULL;

    if (strcmp(action, "rematch_game") == 0) {

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

    if (json_response) {
        server_send(client_socket, json_response);
    } else {
        LOG_WARN("%s\n", "JSON response is empty");
        return;
    }
    
    if (result)             free(result);
    if (json_response)      free(json_response);
    if (action)             free(action); 
    if (notification)       free(notification);
}