#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "../dto/game_dto.h"
#include "../dto/notification_dto.h"
#include "../dto/participation_request_dto.h"
#include "../dto/play_dto.h"
#include "../dto/player_dto.h"
#include "../dto/round_dto.h"

/* === Extract functions === */

char* extract_string_from_json(const char* json_str, const char* key);
int extract_int_from_json(const char* json_str, const char* key);


/* === Serialize functions === */

char* serialize_action_success(const char* action, const char* message);
char* serialize_action_error(const char* action, const char* error_message);
char* serialize_players_to_json(const PlayerDTO* players, size_t count);
char* serialize_games_to_json(const GameDTO* games, size_t count);
char* serialize_notification_to_json(NotificationDTO* in_notification);

#endif