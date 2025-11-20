#ifndef NOTIFICATION_CONTROLLER_H
#define NOTIFICATION_CONTROLLER_H

#include "../dto/notification_dto.h"
#include "../entities/play_entity.h"

typedef enum {
    NOTIFICATION_CONTROLLER_OK = 0,
    // NOTIFICATION_CONTROLLER_INVALID_INPUT,
    // NOTIFICATION_CONTROLLER_NOT_FOUND,
    // NOTIFICATION_CONTROLLER_STATE_VIOLATION,
    // NOTIFICATION_CONTROLLER_DATABASE_ERROR,
    // NOTIFICATION_CONTROLLER_CONFLICT,
    NOTIFICATION_CONTROLLER_FORBIDDEN,
    NOTIFICATION_CONTROLLER_INTERNAL_ERROR
} NotificationControllerStatus;


NotificationControllerStatus notification_rematch_game(int64_t id_game, int64_t id_sender, int64_t id_receiver, NotificationDTO **out_dto);

// ===================== Controllers Helper Functions =====================

NotificationControllerStatus notification_participation_request_cancel(int64_t id_request, int64_t id_sender, NotificationDTO **out_dto);

NotificationControllerStatus notification_new_game(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto);
NotificationControllerStatus notification_game_cancel(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto);
NotificationControllerStatus notification_waiting_game(int64_t id_game, int64_t id_sender, NotificationDTO **out_dto);
NotificationControllerStatus notification_finished_round(int64_t id_round, int64_t id_sender, const char *result, NotificationDTO **out_dto);

// Funzione di utilità per messaggi di errore
const char *return_notification_controller_status_to_string(NotificationControllerStatus status);

#endif