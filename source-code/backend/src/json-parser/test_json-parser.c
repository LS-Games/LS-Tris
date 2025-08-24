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

#include <stdio.h>
#include <json-c/json.h> // Header to access all the functions that json-c offers

void json_c() {

    // Declaring variables
    
    FILE *fp; // To open the JSON document
    char buffer[1024]; // 1024 byte to store the JSON document content 
    struct json_object *parsed_json; // Holds the whole JSON document


    // We need a structure for each field in the JSON document
    struct json_object *nickname, *email, *password, *current_streak, *max_streak, *registration_date, *test_array;

    // If we expect a array, we also need a json_object structure to temporarily hold the array item
    struct json_object *test_array_item;

    size_t test_array_length; // We'll store the array length here


    // Opening the JSON document
    fp = fopen("./src/json-parser/input.json", "r"); // Open the JSON document (we know that our working directory is always /workspaces/LS-Tris/source-code/backend)
    fread(buffer, 1024, 1, fp); // Read the file and put its content in the buffer
    fclose(fp); // Close the JSON document

    // Parsing the JSON document content into JSON object
    parsed_json = json_tokener_parse(buffer);

    // Get the value of a key in the JSON object
    json_object_object_get_ex(parsed_json, "nickname", &nickname);
    json_object_object_get_ex(parsed_json, "email", &email);
    json_object_object_get_ex(parsed_json, "password", &password);
    json_object_object_get_ex(parsed_json, "current_streak", &current_streak);
    json_object_object_get_ex(parsed_json, "max_streak", &max_streak);
    json_object_object_get_ex(parsed_json, "registration_date", &registration_date);
    json_object_object_get_ex(parsed_json, "test_array", &test_array);

    // Convert the json_object into desired type
    printf("Nickname: %s\n", json_object_get_string(nickname));
    printf("Email: %s\n", json_object_get_string(email));
    printf("Password: %s\n", json_object_get_string(password));
    printf("Current Streak: %d\n", json_object_get_int(current_streak));
    printf("Max Streak: %d\n", json_object_get_int(max_streak));
    printf("Registration Date: %s\n", json_object_get_string(registration_date));

    // Array convertion
    test_array_length = json_object_array_length(test_array);
    printf("Found %zu items\n", test_array_length);

    for (int i=0; i<test_array_length; i++) {
        test_array_item = json_object_array_get_idx(test_array, i);
        printf ("%zu: %s\n", i+1, json_object_get_string(test_array_item));
    }
}