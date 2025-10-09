#ifndef NOTIFICATION_DTO_H
#define NOTIFICATION_DTO_H

#include <stdint.h>

typedef struct NotificationDTO {
    int64_t id_playerSender;
    int64_t id_playerReceiver;
    char *message;
    int64_t id_game;
    int64_t id_round;
} NotificationDTO;

#endif