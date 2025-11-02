#include <string.h>
#include <sys/socket.h> // per send()
#include <inttypes.h>

#include "session_manager.h"
#include "../../include/debug_log.h"

void session_manager_init(SessionManager *manager) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer for init is NULL");
        return;
    }

    manager->count = 0;
    pthread_mutex_init(&manager->lock, NULL);

    for (int i = 0; i < MAX_SESSION; i++) {
        manager->list[i].active = 0;
    }
}

void session_add(SessionManager *manager, int fd, int64_t id_player, const char *nickname) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return;
    }

    pthread_mutex_lock(&manager->lock);

    if (manager->count >= MAX_SESSION) {
        LOG_WARN("Cannot add session: session list full (%d active)\n", manager->count);
        pthread_mutex_unlock(&manager->lock);
        return;
    }

    for (int i = 0; i < MAX_SESSION; i++) {
        if (!manager->list[i].active) {
            manager->list[i].fd = fd;
            manager->list[i].id_player = id_player;
            strncpy(manager->list[i].nickname, nickname, sizeof(manager->list[i].nickname) - 1);
            manager->list[i].nickname[sizeof(manager->list[i].nickname) - 1] = '\0';
            manager->list[i].active = 1;
            manager->count++;

            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);
}

void session_remove(SessionManager *manager, int fd) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return;
    }

    pthread_mutex_lock(&manager->lock);

    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].fd == fd && manager->list[i].active) {
            manager->list[i].active = 0;
            manager->count--;
            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);
}

Session *session_find_by_fd(SessionManager *manager, int fd) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return NULL;
    }

    pthread_mutex_lock(&manager->lock);

    Session *result = NULL;
    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].fd == fd && manager->list[i].active) {
            result = &manager->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);
    return result;
}

Session *session_find_by_id_player(SessionManager *manager, int64_t id_player) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return NULL;
    }

    pthread_mutex_lock(&manager->lock);

    Session *result = NULL;
    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].id_player == id_player && manager->list[i].active) {
            result = &manager->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);
    return result;
}

Session *session_find_by_nickname(SessionManager *manager, const char *nickname) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return NULL;
    }

    pthread_mutex_lock(&manager->lock);

    Session *result = NULL;
    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].active && strcmp(manager->list[i].nickname, nickname) == 0) {
            result = &manager->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);
    return result;
}

int session_broadcast(SessionManager *manager, const char *message, int sender_fd) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return -1;
    }

    if (!message || strlen(message) == 0) {
        LOG_WARN("%s\n", "Broadcast message is empty");
        return -1;
    }

    pthread_mutex_lock(&manager->lock);

    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].active && manager->list[i].fd != sender_fd) {
            send(manager->list[i].fd, message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&manager->lock);

    return 0;
}

int session_unicast(SessionManager *manager, const char *message, int receiver_fd) {

    if (!manager) {
        LOG_WARN("%s\n", "SessionManager pointer is NULL");
        return -1;
    }

    if (!message || strlen(message) == 0) {
        LOG_WARN("%s\n", "Unicast message is empty");
        return -1;
    }

    pthread_mutex_lock(&manager->lock);

    for (int i = 0; i < MAX_SESSION; i++) {
        if (manager->list[i].active && manager->list[i].fd == receiver_fd) {
            send(manager->list[i].fd, message, strlen(message), 0);
            break;
        }
    }

    pthread_mutex_unlock(&manager->lock);

    return 0;
}

void print_session_list(SessionManager *manager) {

    if(!manager) {
        return;
    }

    LOG_INFO("%s\n", "Connection List:");
    for(int i=0; i<MAX_SESSION; i++) {
        if(manager->list[i].active == 1) {
            LOG_INFO(" %d) Player ID: %" PRId64 ",\t Nickname: %s,\t fd: %d,\t", i, manager->list[i].id_player, manager->list->nickname, manager->list[i].fd);
        }
    }

}
