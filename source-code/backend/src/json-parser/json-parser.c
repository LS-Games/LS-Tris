#include <json-c/json.h>
#include <string.h>

#include "json-parser.h"

//json_str is the variable which contains the entire JSON
//key is the json key which we will use to extract the correct value from the JSON
char* extract_string_from_json(const char* json_str, const char* key) {

    //parsed_join will contain the entire JSON object after the parsing
    //value will contain the value associated with the key
    struct json_object *parsed_json, *value;                          

    //Convert a string in a json_object*
    parsed_json = json_tokener_parse(json_str);

    if(!parsed_json) return NULL;

    //json_object_get_ex searches in parsed_json using key and put the value in the value variable if it finds him
    //It returns false if it cannot be found, true otherwise
    if(!json_object_object_get_ex(parsed_json, key, &value)) {

        //This function is used to free up the memory allocated from json_tokener_parse() function
        json_object_put(parsed_json);
        return NULL;
    }

    const char *temp = json_object_get_string(value);
    if (!temp) {
        json_object_put(parsed_json);
        return NULL;
    }

    //With +1 we add the space for the string terminator \0
    char *result = malloc(strlen(temp) + 1);
    if (!result) {
        json_object_put(parsed_json);
        return NULL;
    }

    strcpy(result, temp);
    json_object_put(parsed_json);

    return result;
}

int extract_int_from_json(const char* json_str, const char* key) {
    
    struct json_object *parsed_json, *value;

    parsed_json = json_tokener_parse(json_str);

    if(!parsed_json) return -1;

    if(!json_object_object_get_ex(parsed_json, key, &value)) {

        //This function is used to free up the memory allocated from json_tokener_parse() function
        json_object_put(parsed_json);
        return -1;
    }

    int result = json_object_get_int(value);

    json_object_put(parsed_json);

    return result;
}

char* serialize_notification_to_json(NotificationDTO* in_notification) {

    struct json_object *json_response = json_object_new_object();

    json_object_object_add(json_response, "id_sender", json_object_new_int64(in_notification->id_playerSender));
    json_object_object_add(json_response, "id_receiver", json_object_new_int64(in_notification->id_playerReceiver));
    json_object_object_add(json_response, "message", json_object_new_string(in_notification->message));
    json_object_object_add(json_response, "id_game", json_object_new_int64(in_notification->id_game));
    json_object_object_add(json_response, "id_round", json_object_new_int64(in_notification->id_round));

    const char* json_str = json_object_to_json_string(json_response);

    char* result = malloc(strlen(json_str) + 1);

    if (result) strcpy(result, json_str);

    json_object_put(json_response); 
    return result;     
}