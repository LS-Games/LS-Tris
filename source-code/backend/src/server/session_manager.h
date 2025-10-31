#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <pthread.h>
#include <stdint.h>


#define MAX_SESSION 100

//This struct rapresents the single user session  
 typedef struct {

    int fd;
    int64_t player_id;
    char nickname[64];
    int active;
} Session;

/**
 * This struct rapresents the user session container.
 * In this struct we have a mutex which provides a secure access to resource since
 * we have multiple threads that can modify the list simultaneosuly
 */

typedef struct {

    Session list[MAX_SESSION];      //We use array because we have a few sessions (We should use different structures)
    int count;                      //Active session number
    pthread_mutex_t lock;           
} SessionManager;

void session_manager_init(SessionManager *manager);
void session_add(SessionManager *manager, int fd, int64_t player_id, const char *nickname);
void session_remove(SessionManager *manager, int fd);
Session *session_find_by_fd(SessionManager *manager, int fd);
Session *session_find_by_nickname(SessionManager *manager, const char* nickname);
void session_broadcast(SessionManager *manager, const char *message, int exclude_fd);

void print_session_list(SessionManager *manager);

//Using a extern variabile we can use the same istance in .c different files
extern SessionManager session_manager;
#endif