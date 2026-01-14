#ifndef SERVE_H
#define SERVER_H

#include <inttypes.h>

#define MAX_CLIENTS 100

int start_server(int port);
int send_server_response(int client_socket, const char* data);
int send_server_broadcast_message(const char *message, int64_t id_sender);
int send_server_unicast_message(const char *message, int64_t id_receiver);
int send_framed_json(int fd, const char *json);

#endif