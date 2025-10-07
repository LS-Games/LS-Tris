#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "notification_dto.h"

const char* extract_string_from_json(const char* json_str, const char* key);
int extract_int_from_json(const char* json_str, const char* key);

char* serialize_notification_to_json(NotificationDTO* notification);

#endif