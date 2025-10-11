#ifndef ROUTER_H
#define ROUTER_H

void route_request(const char* path, const char* json_body, int client_socket);

#endif