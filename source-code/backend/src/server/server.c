#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "../../include/debug_log.h"
#include "./session_manager.h"

#include "server.h"
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


    session_manager_init(&session_manager);
    LOG_INFO("%s\n", "Session manager initialized. Ready to accept clients.");

    // Infinite loops continue to accept clients 
    while(1) {
        // For each iteration we will have a new address in the heap, in the handle_client function we reead
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

// This functino named "function worker" handles a single client
// @param *arg Corresponds to file descriptor that we will pass to it 
// @return A void* because the thread POSIX standard wants this signature
static void *handle_client(void *arg) {

    // When we call accept() we dinamicaly allocate an int using malloc 
    // We do a cast in a int pointer and then we dereference it to obtain the numerical value. 
    int client_fd = *(int*)arg; 

    // We free up the allocated memory after saving it
    free(arg); 

    // The buffer is used for to store data arriving from client 
    char buffer[1024];
    ssize_t n;
    ssize_t total = 0 ;
    char *accumulated = NULL;
    int persistence = 0;

    // We have to use while because the TCP connection is a stream, we may receive fragmented message
    // recv return a 0 if the connection is interrupted, -1 in case of errors and n which are the bytes received 
    while((n = recv(client_fd, buffer, sizeof(buffer)-1, 0)) > 0) {

        // We added the string terminator so we can use it as a string in C
        buffer[n] = '\0';
        
        char *new_accum = realloc(accumulated, total + n + 1);

        if (!new_accum) {
            LOG_ERROR("%s\n", "Realloc error");
            free(accumulated);
            close(client_fd);
            break;
        }

        accumulated = new_accum;

        // Start from total value and copy what's inside buffer which has length n
        memcpy(&accumulated[total], buffer, n);
        total += n;
        accumulated[total] = '\0';

        char *terminator = strstr(accumulated, "\r\n\r\n");

        if (terminator != NULL) {
            *terminator = '\0';  

            LOG_INFO("Full message received: %s\n", accumulated);

            route_request(accumulated, client_fd, &persistence);

            free(accumulated);
            accumulated = NULL;
            total = 0;

            //This is the case of SignIn action that's why we're closing the connection immediately
            if (persistence == 0) {
                LOG_INFO("Closing connection with fd=%d\n", client_fd);
                close(client_fd);
                return NULL;
            }
        }
    }

    if (n == 0) {

        /**
         * When we receive n==0 it means that the connection has been closed 
         * This is the case where the client:
         * closes the browser windows -> the bridge captures the event and closes the conncetion with the backend
         * the recv function receives 0 as value and call the manager function to delete the connection from the list
         */

        session_remove(&session_manager, client_fd);
        LOG_INFO("Client fd=%d closed the connection\n", client_fd);
        print_session_list(&session_manager);


    } else if (n < 0) {
        LOG_ERROR("recv() failed for fd=%d\n", client_fd);
    }

    free(accumulated);
    close(client_fd);
    return NULL;
}

// NB: We're using recv and send functions instead of read and write because we're working with socket


int server_send(int client_socket, const char *data) {

    if(client_socket < 0 || !data) {
        LOG_WARN("Invalid parameters: socket = %d, data = %p\n", client_socket, (void*)data);
        return -1;
    }

    ssize_t sent = send(client_socket, data, strlen(data), 0);

    if (sent < 0) {
        LOG_ERROR("Failed to send data to client socket %d\n", client_socket);
        return -1;
    }

    return sent;
}
