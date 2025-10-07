#include "json-parser.h"
#include "notification_dto.h"
#include "controllers/notification_controller.h"
#include <string.h>
#include <sys/socket.h> 

void route_request(const char* json_body, int client_socket) {

    const char* action = extract_string_from_json(json_body, "action");
    int id_game = extract_int_from_json(json_body, "id_game");
    int id_sender = extract_int_from_json(json_body, "id_sender");
    int id_receiver = extract_int_from_json(json_body, "id_receiver");

    NotificationDTO *notification;

    if (strcmp(action, "rematch_game") == 0) {
       if(notification_rematch_game(id_game, id_sender, id_receiver, &notification) == NOTIFICATION_CONTROLLER_OK) {
            char* json_response = serialize_notification_to_json(notification);
            send(client_socket, json_response, strlen(json_response), 0);

            free(json_response);
            free(notification);
       }
        
    }

    free(action); 
}