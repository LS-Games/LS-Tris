/**
 * The json parser library we used is json-c.
 * GitHub repository: https://github.com/json-c/json-c 
 * Reference: https://json-c.github.io/json-c/
 * 
 * You can verify the installed libjson-c version with the command: `dpkg -l | grep libjson-c`
 * Our version is the json-c 16.0. 
 * json-c 0.16 reference: https://json-c.github.io/json-c/json-c-0.16/doc/html/index.html
 * Video tutorial: https://youtu.be/dQyXuFWylm4?si=KWfLu5QcM055VKwl
*/

#include <json-c/json.h> // Header to access all the functions that json-c offers
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

    // Convert the json_object into desired type
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

    // Parsing the JSON string content into JSON object
    parsed_json = json_tokener_parse(json_str);

    if(!parsed_json) return -1;

    // Get the value of a key in the JSON object
    if(!json_object_object_get_ex(parsed_json, key, &value)) {

        // This function is used to free up the memory allocated from json_tokener_parse() function
        json_object_put(parsed_json);
        return -1;
    }

    // Convert the json_object into desired type
    int result = json_object_get_int(value);

    json_object_put(parsed_json);

    return result;
}

ParticipationRequest* extract_requests_array_from_json(const char *json_str, size_t *out_count) {

     if (!out_count) return NULL;

    struct json_object *parsed_json = json_tokener_parse(json_str);
    if (!parsed_json) return NULL;

    struct json_object *requests_array;

    if (!json_object_object_get_ex(parsed_json, "requests", &requests_array)) {
        json_object_put(parsed_json);
        *out_count = 0;
        return NULL;
    }

    if (!json_object_is_type(requests_array, json_type_array)) {
        json_object_put(parsed_json);
        *out_count = 0;
        return NULL;
    }

    int len = json_object_array_length(requests_array);
    *out_count = len;

    ParticipationRequest *buffer =
        malloc(sizeof(ParticipationRequest) * len);

    for (int i = 0; i < len; i++) {
        struct json_object *item = json_object_array_get_idx(requests_array, i);

        buffer[i].id_request = json_object_get_int64(json_object_object_get(item, "id_request"));
        buffer[i].id_player  = json_object_get_int64(json_object_object_get(item, "id_player"));
        buffer[i].id_game    = json_object_get_int64(json_object_object_get(item, "id_game"));

        const char *state = json_object_get_string(json_object_object_get(item, "state"));
        buffer[i].state = string_to_request_participation_status(state);
    }

    json_object_put(parsed_json);
    return buffer;
}

/* === Serialize functions === */

// Serialize: Action Success
char *serialize_action_success(const char *action, const char *message, int64_t id) {
    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "status", json_object_new_string("success"));

    if (id != -1) {
        json_object_object_add(json_response, "id", json_object_new_int64(id));
    }
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

// Serialize: Action Success (with waiting flag)
char *serialize_action_success_with_waiting(const char *action, const char *message, int64_t id, int waiting) {
    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "status", json_object_new_string("success"));

    if (id != -1) {
        json_object_object_add(json_response, "id", json_object_new_int64(id));
    }
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    if (message) {
        json_object_object_add(json_response, "message", json_object_new_string(message));
    }

    // waiting: 1 if the caller must wait for the opponent, 0 otherwise
    json_object_object_add(json_response, "waiting", json_object_new_int(waiting));

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
char *serialize_players_to_json(const char *action, const PlayerDTO* players, size_t count) {
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

    json_object_object_add(json_response, "status", json_object_new_string("success"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
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
char *serialize_games_to_json(const char *action, const GameDTO* games, size_t count) {
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

    json_object_object_add(json_response, "status", json_object_new_string("success"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "games", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

//Serialize: GameDTO with streaks
char *serialize_games_with_streak_to_json(const char *action, const GameDTO *games, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {

        struct json_object *json_game = json_object_new_object();

        json_object_object_add(json_game, "id_game",
            json_object_new_int64(games[i].id_game));

        json_object_object_add(json_game, "creator_nickname",
            json_object_new_string(games[i].creator_nickname ? games[i].creator_nickname : ""));

        json_object_object_add(json_game, "owner_nickname",
            json_object_new_string(games[i].owner_nickname ? games[i].owner_nickname : ""));

        json_object_object_add(json_game, "state",
            json_object_new_string(games[i].state_str ? games[i].state_str : ""));

        json_object_object_add(json_game, "created_at",
            json_object_new_string(games[i].created_at_str ? games[i].created_at_str : ""));

        /* --- streak info --- */
        if (games[i].owner_current_streak >= 0) {
            json_object_object_add(json_game,
                "owner_current_streak",
                json_object_new_int(games[i].owner_current_streak));
        }

        if (games[i].owner_max_streak >= 0) {
            json_object_object_add(json_game,
                "owner_max_streak",
                json_object_new_int(games[i].owner_max_streak));
        }

        json_object_array_add(json_array, json_game);
    }

    json_object_object_add(json_response, "status", json_object_new_string("success"));

    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }

    json_object_object_add(json_response, "count", json_object_new_int64((int64_t)count));
    json_object_object_add(json_response, "games", json_array);

    const char *json_str = json_object_to_json_string(json_response);

    char *result = malloc(strlen(json_str) + 1);
    if (!result) {
        json_object_put(json_response);
        return NULL;
    }

    strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

char *serialize_game_updated_to_json(const GameDTO *game) {
    if (!game) return NULL;

    struct json_object *json_response = json_object_new_object();
    struct json_object *json_game = json_object_new_object();

    json_object_object_add(json_game, "id_game",
        json_object_new_int64(game->id_game));

    json_object_object_add(json_game, "creator_nickname",
        json_object_new_string(game->creator_nickname));

    json_object_object_add(json_game, "owner_nickname",
        json_object_new_string(game->owner_nickname));

    json_object_object_add(json_game, "owner_current_streak",
        json_object_new_int(game->owner_current_streak));

    json_object_object_add(json_game, "owner_max_streak",
        json_object_new_int(game->owner_max_streak));

    json_object_object_add(json_game, "state",
        json_object_new_string(game->state_str));

    json_object_object_add(json_game, "created_at",
        json_object_new_string(game->created_at_str));

    json_object_object_add(json_response, "status",
        json_object_new_string("success"));

    json_object_object_add(json_response, "action",
        json_object_new_string("server_game_updated"));

    json_object_object_add(json_response, "game", json_game);

    const char *json_str = json_object_to_json_string(json_response);

    char *result = malloc(strlen(json_str) + 1);
    if (result)
        strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: RoundDTO
char *serialize_rounds_to_json(const char *action, const RoundDTO* rounds, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_round = json_object_new_object();

        json_object_object_add(json_round, "id_round",
            json_object_new_int64(rounds[i].id_round));

        json_object_object_add(json_round, "id_game",
            json_object_new_int64(rounds[i].id_game));

        json_object_object_add(json_round, "state",
            json_object_new_string(rounds[i].state_str));

        json_object_object_add(json_round, "start_time",
            json_object_new_int64(rounds[i].start_time));

        json_object_object_add(json_round, "end_time",
            json_object_new_int64(rounds[i].end_time));

        // Duration is DERIVED, not stored
        if (rounds[i].end_time > 0 && rounds[i].start_time > 0) {
            json_object_object_add(json_round, "duration",
                json_object_new_int64(rounds[i].end_time - rounds[i].start_time));
        }

        json_object_object_add(json_round, "board",
            json_object_new_string(rounds[i].board));

        json_object_array_add(json_array, json_round);
    }

    json_object_object_add(json_response, "status",
        json_object_new_string("success"));

    if (action) {
        json_object_object_add(json_response, "action",
            json_object_new_string(action));
    }

    json_object_object_add(json_response, "count",
        json_object_new_int64(count));

    json_object_object_add(json_response, "rounds", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}


// Serialize: ParticipationRequestDTO
char *serialize_participation_requests_to_json(const char *action, const ParticipationRequestDTO* participationRequests, size_t count) {
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

    json_object_object_add(json_response, "status", json_object_new_string("success"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "participation_requests", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);

    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

// Serialize: PlayDTO
char *serialize_plays_to_json(const char *action, const PlayDTO* plays, size_t count) {
    struct json_object *json_response = json_object_new_object();
    struct json_object *json_array = json_object_new_array();

    for (size_t i = 0; i < count; i++) {
        struct json_object *json_play = json_object_new_object();

        json_object_object_add(json_play, "id_player", json_object_new_int64(plays[i].id_player));
        json_object_object_add(json_play, "id_round", json_object_new_int64(plays[i].id_round));
        json_object_object_add(json_play, "player_number", json_object_new_int(plays[i].player_number));
        json_object_object_add(json_play, "player_nickname", json_object_new_string(plays[i].player_nickname));
        json_object_object_add(json_play, "result", json_object_new_string(plays[i].result_str));

        json_object_array_add(json_array, json_play);
    }

    json_object_object_add(json_response, "status", json_object_new_string("success"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    json_object_object_add(json_response, "count", json_object_new_int64(count));
    json_object_object_add(json_response, "plays", json_array);

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response);
    return result;
}

char *serialize_notification_to_json(const char *action, NotificationDTO* in_notification) {

    if (!in_notification) return NULL;

    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "status", json_object_new_string("success"));
    if (action) {
        json_object_object_add(json_response, "action", json_object_new_string(action));
    }
    if (in_notification->id_playerSender != -1) {
        json_object_object_add(json_response, "id_sender", json_object_new_int64(in_notification->id_playerSender));
    }
    if (in_notification->id_playerReceiver != -1) {
        json_object_object_add(json_response, "id_receiver", json_object_new_int64(in_notification->id_playerReceiver));
    }
    if (in_notification->id_game != -1) {
        json_object_object_add(json_response, "id_game", json_object_new_int64(in_notification->id_game));
    }
    if (in_notification->id_round != -1) {
        json_object_object_add(json_response, "id_round", json_object_new_int64(in_notification->id_round));
    }

    if (in_notification->id_request != -1) {
        json_object_object_add(json_response, "id_request", json_object_new_int64(in_notification->id_request));
    }
    
    json_object_object_add(json_response, "message", json_object_new_string(in_notification->message));

    const char *json_str = json_object_to_json_string(json_response);
    char *result = malloc(strlen(json_str) + 1);
    if (result) strcpy(result, json_str);

    json_object_put(json_response); 

    return result;     
}

char *serialize_round_full_to_json(const char *action, RoundFullDTO* in_round_full) {

    if (!in_round_full || !action) return NULL;

    json_object *root = json_object_new_object();
    if (!root) return NULL;

    json_object_object_add(root, "status", json_object_new_string("success"));
    json_object_object_add(root, "action", json_object_new_string(action));

    json_object *round_obj = json_object_new_object();

    json_object_object_add(round_obj, "id_round",
        json_object_new_int64(in_round_full->id_round));

    json_object_object_add(round_obj, "id_game",
        json_object_new_int64(in_round_full->id_game));

    json_object_object_add(round_obj, "state",
        json_object_new_string(in_round_full->state));

    json_object_object_add(round_obj, "start_time",
        json_object_new_int64(in_round_full->start_time));

    json_object_object_add(round_obj, "end_time",
        json_object_new_int64(in_round_full->end_time));

    // Duration is derived, not persisted
    if (in_round_full->start_time > 0 && in_round_full->end_time > 0) {
        json_object_object_add(round_obj, "duration",
            json_object_new_int64(in_round_full->end_time - in_round_full->start_time));
    }

    json_object_object_add(round_obj, "board",
        json_object_new_string(in_round_full->board));

    json_object_object_add(round_obj, "id_player1",
        json_object_new_int64(in_round_full->id_player1));
    json_object_object_add(round_obj, "id_player2",
        json_object_new_int64(in_round_full->id_player2));

    json_object_object_add(round_obj, "nickname_player1",
        json_object_new_string(in_round_full->nickname_player1));
    json_object_object_add(round_obj, "nickname_player2",
        json_object_new_string(in_round_full->nickname_player2));

    json_object_object_add(round_obj, "player_number_player1",
        json_object_new_int(in_round_full->player_number_player1));
    json_object_object_add(round_obj, "player_number_player2",
        json_object_new_int(in_round_full->player_number_player2));

    json_object_object_add(root, "round", round_obj);

    const char *json_str = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PLAIN);
    if (!json_str) {
        json_object_put(root);
        return NULL;
    }

    size_t len = strlen(json_str);

    char *result = malloc(len + 1);
    if (!result) {
        json_object_put(root);
        return NULL;
    }

    memcpy(result, json_str, len + 1);

    json_object_put(root);

    return result;
}
