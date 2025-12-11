#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "../../include/debug_log.h"

#include "server.h"
#include "session_manager.h"
#include "router.h"

SessionManager session_manager;

// ==================== Private functions ====================

static void *handle_client(void *arg);

// ===========================================================

// This function starts the server
int start_server(int port) {
    
    int server_fd;
    // This structure provides us with a way to describe a IPv4 (is provided by netinet.h library)
    struct sockaddr_in addr;

    // With socket() function we're creating a new sockest, it returns a: 
    // integer < 0 in case of errors or a integer >= 0 which rapresents the assigned file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;              // It represents IPv4
    addr.sin_addr.s_addr = INADDR_ANY;      // It means "listen on all newtwork interfaces"
    addr.sin_port = htons(port);            // The listening port 

    // bind() creates a connection between the server file description and the port in addr variable
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    // listen() function puts the socket in passive mode (listening mode)
    if(listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    LOG_INFO("Server listens on port: %d... \n", port);


    //Session Manager keeps track of the active sessions
    session_manager_init(&session_manager);
    LOG_INFO("%s\n", "Session manager initialized. Ready to accept clients.");

    // Infinite loops continue to accept clients 
    while(1) {
        // For each iteration we will have a new address in the heap, in the handle_client function we read
        // this value and then we relase him using free()
        int *client_fd = malloc(sizeof(int));

        // With accept() function we get out the estabilished connection in the queue 
        *client_fd = accept(server_fd, NULL, NULL);

        if(*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        // We can use a local variable because pthread_create() function assign to him a new tid every time
        // handle_client is the function executed by the thread, client_fd the handle_client parameter
        int errorNumber;
        pthread_t tid;
        if ((errorNumber = pthread_create(&tid, NULL, handle_client, client_fd) != 0)) {
            errno=errorNumber;
            perror("pthread_create");
            continue;
        }
        
        // pthread_detach() marks the thread as "detached"
        // This means that when the thread finishes, its resources are released automatically
        // Therefore the parent thread does not need to call pthread_join() to clean up
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}

// Reads exactly that byte from the socket (or fails)
// Returns: 1 = OK, 0 = connection closed, -1 = error
static int recv_all(int fd, void *buf, size_t len) {

    char *p = buf;
    size_t remaining = len;

    while (remaining > 0) {
        ssize_t n = recv(fd, p, remaining, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) {
            // peer has closed the connection
            return 0;
        }

        //Let's move forward in the buffer
        p += n;
        //We calculate the remainder
        remaining -= n;
    }

    return 1;
}

// NB: We're using recv and send functions instead of read and write because we're working with socket

// "worker function" that manages a single client 
static void *handle_client(void *arg) {

    int client_fd = *(int*)arg;
    free(arg);

    while (1) {

        //I read the 4-byte header (it contains the length of the JSON)
        uint32_t len_net = 0;

        int r = recv_all(client_fd, &len_net, sizeof(len_net));

        if (r == 0) {
            // Il client ha chiuso la connessione
            LOG_INFO("Client fd=%d closed the connection (while reading header)\n", client_fd);
            break;
        } else if (r < 0) {
            LOG_ERROR("recv_all() failed on header for fd=%d\n", client_fd);
            break;
        }

        uint32_t len = ntohl(len_net);

        if (len == 0) {
            LOG_WARN("Received empty frame from fd=%d\n", client_fd);
            continue; // ignoro, ma tengo aperta la connessione
        }

        // (opzionale) limite massimo per sicurezza, es. 1MB
        if (len > 1024 * 1024) {
            LOG_WARN("Frame too large (%u bytes) from fd=%d\n", len, client_fd);
            break;
        }

        // 2) Alloco il buffer per il JSON
        char *json_body = malloc(len + 1);
        if (!json_body) {
            LOG_ERROR("malloc() failed for JSON body (len=%u) fd=%d\n", len, client_fd);
            break;
        }

        // 3) Leggo il corpo JSON
        r = recv_all(client_fd, json_body, len);
        if (r == 0) {
            LOG_INFO("Client fd=%d closed the connection (while reading body)\n", client_fd);
            free(json_body);
            break;
        } else if (r < 0) {
            LOG_ERROR("recv_all() failed on body for fd=%d\n", client_fd);
            free(json_body);
            break;
        }

        // Termino la stringa
        json_body[len] = '\0';

        LOG_INFO("Full message received: %s\n", json_body);

        int persistence = 1; // di default la teniamo aperta
        route_request(json_body, client_fd, &persistence);

        free(json_body);

        // Se la route ci dice che è una connessione non persistente, chiudiamo
        if (persistence == 0) {
            LOG_INFO("Closing connection with fd=%d (non-persistent)\n", client_fd);
            break;
        }
    }

    // Rimuovo la sessione se esiste
    session_remove(&session_manager, client_fd);
    LOG_INFO("Client fd=%d closed the connection\n", client_fd);
    print_session_list(&session_manager);

    close(client_fd);
    return NULL;
}

static int send_all(int fd, const void *buf, size_t len) {
    const char *p = buf;
    while (len > 0) {
        ssize_t n = send(fd, p, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue; // ritenta se segnale
            return -1;
        }
        if (n == 0) return -1; // connessione chiusa
        p   += n;
        len -= n;
    }
    return 0;
}

int send_framed_json(int fd, const char *json) {

    if (!json) return -1;
    uint32_t len = (uint32_t)strlen(json);
    uint32_t len_net = htonl(len);

    if (send_all(fd, &len_net, sizeof(len_net)) < 0) {
        LOG_ERROR("Failed to send length header to fd=%d\n", fd);
        return -1;
    }

    if (send_all(fd, json, len) < 0) {
        LOG_ERROR("Failed to send JSON body to fd=%d\n", fd);
        return -1;
    }
    return 0;
}


int send_server_response(int client_socket, const char *data) {

    if(client_socket < 0 || !data) {
        LOG_WARN("Invalid parameters: socket = %d, data = %p\n",
                 client_socket, (void*)data);
        return -1;
    }

    if (send_framed_json(client_socket, data) < 0) {
        LOG_ERROR("Failed to send framed JSON to client socket %d\n", client_socket);
        return -1;
    }

    return 0;
}



int send_server_broadcast_message(const char *message, int64_t id_sender) {

    Session session_sender;

    if(!(session_find_by_id_player(&session_manager, id_sender, &session_sender))) {
        LOG_WARN("Session not found");
        return -1;
    }

    if (session_broadcast(&session_manager, message, session_sender.fd) < 0) {
        LOG_WARN("Error in sending broadcast message from sender %" PRId64 "\n", id_sender);
        return -1;
    } else {
        LOG_DEBUG("Sent message from sender %" PRId64 ": %s\n", id_sender, message);
    }

    return 0;
}

int send_server_unicast_message(const char *message, int64_t id_receiver) {

    Session receiverSession;  

    if (!(session_find_by_id_player(&session_manager, id_receiver, &receiverSession))) {
        LOG_WARN("Receiver session not found\n");
        return -1;
    }

    if(receiverSession.fd < 1) {
        LOG_WARN("Session fd is negative");
        return -1;
    }

    int fd = receiverSession.fd; 
    return session_unicast(&session_manager, message, fd);
}
