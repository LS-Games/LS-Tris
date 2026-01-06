#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "../dto/game_dto.h"
#include "../dto/notification_dto.h"
#include "../dto/participation_request_dto.h"
#include "../dto/play_dto.h"
#include "../dto/player_dto.h"
#include "../dto/round_dto.h"


/* === Extract functions === */

char *extract_string_from_json(const char *json_str, const char *key);
int extract_int_from_json(const char *json_str, const char *key);
ParticipationRequest* extract_requests_array_from_json(const char *json_str, size_t *out_count);


/* === Serialize functions === */

char *serialize_action_success(const char *action, const char *message, int64_t id);
char *serialize_action_success_with_waiting(const char *action, const char *message, int64_t id, int waiting);
char *serialize_action_error(const char *action, const char *error_message);
char *serialize_players_to_json(const char *action, const PlayerDTO* players, size_t count);
char *serialize_games_to_json(const char *action, const GameDTO* games, size_t count);
char *serialize_games_with_streak_to_json(const char *action, const GameDTO *games, size_t count);
char *serialize_rounds_to_json(const char *action, const RoundDTO* rounds, size_t count);
char *serialize_participation_requests_to_json(const char *action, const ParticipationRequestDTO* participationRequests, size_t count);
char *serialize_plays_to_json(const char *action, const PlayDTO* plays, size_t count);
char *serialize_notification_to_json(const char *action, NotificationDTO* in_notification);
char *serialize_round_full_to_json(const char *action, RoundFullDTO* in_round_full);

#endif