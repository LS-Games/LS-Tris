#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <pthread.h>
#include <stdint.h>


#define MAX_SESSION 100

// This struct rapresents the single user session  
 typedef struct {
    int fd;
    int64_t id_player;
    char nickname[64];
    int active;
} Session;


/**
 * This struct rapresents the user session container.
 * In this struct we have a mutex which provides a secure access to resource since
 * we have multiple threads that can modify the list simultaneosuly
 */
typedef struct {
    Session list[MAX_SESSION];      // We use array because we have a few sessions (We should use different structures)
    int count;                      // Active session number
    pthread_mutex_t lock;           // Mutex "by structure"
} SessionManager;

// ===================== Session management =====================

void session_manager_init(SessionManager *manager);
void session_add(SessionManager *manager, int fd, int64_t id_player, const char *nickname);
void session_remove(SessionManager *manager, int fd);

// ===================== Find session =====================

int session_find_by_fd(SessionManager *manager, int fd, Session *out);
int session_find_by_id_player(SessionManager *manager, int64_t id_player, Session *out);
int session_find_by_nickname(SessionManager *manager, const char* nickname, Session *out);

// ===================== Message sender =====================

int session_broadcast(SessionManager *manager, const char *message, int sender_fd);
int session_unicast(SessionManager *manager, const char *message, int receiver_fd);

// ===================== Utilities =====================

void print_session_list(SessionManager *manager);

// Using a extern variabile we can use the same istance in different .c files
extern SessionManager session_manager;
#endif