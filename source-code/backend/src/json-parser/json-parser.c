#include <json-c/json.h>
#include <string.h>

#include "json-parser.h"

/* === Extract functions === */

// @param json_str is the variable which contains the entire JSON
// @param key is the json key which we will use to extract the correct value from the JSON
// @return The value extracted from the json key. It returns `NULL` if no key is found.
char *extract_string_from_json(const char *json_str, const char *key) {

    // - parsed_join will contain the entire JSON object after the parsing
    // - value will contain the value associated with the key
    struct json_object *parsed_json, *value;                          

    // Convert a string in a json_object*
    parsed_json = json_tokener_parse(json_str);

    if(!parsed_json) return NULL;

    // json_object_get_ex searches in parsed_json using key and put the value in the value variable if it finds him
    // It returns false if it cannot be found, true otherwise
    if(!json_object_object_get_ex(parsed_json, key, &value)) {

        // This function is used to free up the memory allocated from json_tokener_parse() function
        json_object_put(parsed_json);
        return NULL;
    }

    const char *temp = json_object_get_string(value);
    if (!temp) {
        json_object_put(parsed_json);
        return NULL;
    }

    // With +1 we add the space for the string terminator \0
    char *result = malloc(strlen(temp) + 1);
    if (!result) {
        json_object_put(parsed_json);
        return NULL;
    }

    strcpy(result, temp);
    json_object_put(parsed_json);

    return result;
}

// @param json_str is the variable which contains the entire JSON
// @param key is the json key which we will use to extract the correct value from the JSON
// @return The value extracted from the json key. It returns `-1` if no key is found.
int extract_int_from_json(const char *json_str, const char *key) {
    
    struct json_object *parsed_json, *value;

    parsed_json = json_tokener_parse(json_str);

    if(!parsed_json) return -1;

    if(!json_object_object_get_ex(parsed_json, key, &value)) {

        // This function is used to free up the memory allocated from json_tokener_parse() function
        json_object_put(parsed_json);
        return -1;
    }

    int result = json_object_get_int(value);

    json_object_put(parsed_json);

    return result;
}


/* === Serialize functions === */

// Serialize: Action Success
char *serialize_action_success(const char *action, const char *message, int64_t id) {
    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "status", json_object_new_string("success"));

    json_object_object_add(json_response, "id", json_object_new_int64(id));

    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    if (message) {
        json_object_object_add(json_response, "message", json_object_new_string(message));
    }

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: Action Error
char *serialize_action_error(const char *action, const char *error_message) {
    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "status", json_object_new_string("error"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    if (error_message) {
        json_object_object_add(json_response, "error_message", json_object_new_string(error_message));
    }

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: PlayerDTO
char *serialize_players_to_json(const PlayerDTO* players, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_player = json_object_new_object();

        json_object_object_add(json_player, "id_player", json_object_new_int64(players[i].id_player));
        json_object_object_add(json_player, "nickname", json_object_new_string(players[i].nickname));
        json_object_object_add(json_player, "current_streak", json_object_new_int(players[i].current_streak));
        json_object_object_add(json_player, "max_streak", json_object_new_int(players[i].max_streak));
        json_object_object_add(json_player, "registration_date", json_object_new_string(players[i].registration_date_str));

        json_object_array_add(json_array, json_player);
    }

    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "players", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: GameDTO
char *serialize_games_to_json(const GameDTO* games, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_game = json_object_new_object();

        json_object_object_add(json_game, "id_game", json_object_new_int64(games[i].id_game));
        json_object_object_add(json_game, "creator_nickname", json_object_new_string(games[i].creator_nickname));
        json_object_object_add(json_game, "owner_nickname", json_object_new_string(games[i].owner_nickname));
        json_object_object_add(json_game, "state", json_object_new_string(games[i].state_str));
        json_object_object_add(json_game, "created_at", json_object_new_string(games[i].created_at_str));

        json_object_array_add(json_array, json_game);
    }

    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "games", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: RoundDTO
char *serialize_rounds_to_json(const RoundDTO* rounds, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_round = json_object_new_object();

        json_object_object_add(json_round, "id_round", json_object_new_int64(rounds[i].id_round));
        json_object_object_add(json_round, "id_game", json_object_new_int64(rounds[i].id_game));
        json_object_object_add(json_round, "duration", json_object_new_int64(rounds[i].duration));
        json_object_object_add(json_round, "state", json_object_new_string(rounds[i].state_str));
        json_object_object_add(json_round, "board", json_object_new_string(rounds[i].board));

        json_object_array_add(json_array, json_round);
    }

    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "rounds", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}


// Serialize: ParticipationRequestDTO
char *serialize_participation_requests_to_json(const ParticipationRequestDTO* participationRequests, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_request = json_object_new_object();

        json_object_object_add(json_request, "id_request", json_object_new_int64(participationRequests[i].id_request));
        json_object_object_add(json_request, "id_game", json_object_new_int64(participationRequests[i].id_game));
        json_object_object_add(json_request, "player_nickname", json_object_new_string(participationRequests[i].player_nickname));
        json_object_object_add(json_request, "state", json_object_new_string(participationRequests[i].state_str));
        json_object_object_add(json_request, "created_at", json_object_new_string(participationRequests[i].created_at_str));

        json_object_array_add(json_array, json_request);
    }

    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "participation_requests", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}


char *serialize_notification_to_json(NotificationDTO* in_notification) {
    if (!in_notification) return NULL;

    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "id_sender", json_object_new_int64(in_notification->id_playerSender));
    json_object_object_add(json_response, "id_receiver", json_object_new_int64(in_notification->id_playerReceiver));
    json_object_object_add(json_response, "message", json_object_new_string(in_notification->message));
    json_object_object_add(json_response, "id_game", json_object_new_int64(in_notification->id_game));
    json_object_object_add(json_response, "id_round", json_object_new_int64(in_notification->id_round));

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response); 
    return result;     
}