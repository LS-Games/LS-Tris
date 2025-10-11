#ifndef SERVE_H
#define SERVER_H

#define PORT 55511
#define MAX_CLIENTS 100

int start_server(int port);
int server_send(int client_socket, const char* data);

#endif