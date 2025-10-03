#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

//This functino named "function worker" handles a single client
//It returns void* because the thread POSIX standard wants this signature
//The parameter *arg corresponds to file descriptor that we will pass to it 
void *handle_client(void *arg) {

    //When we call accept() we dinamicalyy allocate an int using malloc 
    //We do a cast in a int pointer and then We dereference it to obtain the numerical value. 
    int client_fd = *(int*)arg; 

    //We free up the allocated memory after saving it
    free(arg); 

    //The buffer is used for to store data arriving from client 
    char buffer[1024];
    int n;

    //We have to use while because the TCP connection is a stream, we may receive fragmented message
    //recv return a 0 if the connection is interrupted, -1 in case of errors and n which are the bytes received 
    while((n = recv(client_fd, buffer, sizeof(buffer)-1, 0)) > 0) {

        //We added the string terminator so we can use it as a string in C
        buffer[n] = '\0';
        printf("Message was received correctly: %s/n", buffer);

        char reply[1024];
        snprintf(reply, sizeof(reply), "Server has received %s:", buffer);

        //With send we can reply to the client 
        send(client_fd, reply, strlen(reply), 0);

        //NB: We're using recv and send functions instead of read and write because we're working with socket
    }

    printf("Client disconnected");
    close(client_fd);
    return NULL;
}

//This function starts the server
int start_server(int port) {
    int server_fd;
    //This structure provides us with a way to describe a IPv4 (is provided by netinet.h library)
    struct sockaddr_in addr;

    //With socket() function we're creating a new sockest, it returns a: 
    //integer < 0 in case of errors or a integer >= 0 which rapresents the assigned file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;              //It represents IPv4
    addr.sin_addr.s_addr = INADDR_ANY;      //It means "listen on all newtwork interfaces"
    addr.sin_port = htons(port);            //The listening port 

    //bind() creates a connection between the server file description and the port in addr variable
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    //listen() function puts the socket in passive mode (listening mode)
    if(listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    printf("Server listens on port: %d ... \n", port);

    //Infinite loops continue to accept clients 
    while(1) {
        //For each iteration we will have a new address in the heap, in the handle_client function we reead
        //this value and then we relase him using free()
        int *client_fd = malloc(sizeof(int));

        //With accept() function we get out the estabilished connection in the queue 
        *client_fd = accept(server_fd, NULL, NULL);

        if(*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        //We can use a local variable because pthread_create() function assign to him a new tid every time
        pthread_t tid; 
        pthread_create(&tid, NULL, handle_client, client_fd);
        
        // pthread_detach() marks the thread as "detached"
        // This means that when the thread finishes, its resources are released automatically
        // Therefore the parent thread does not need to call pthread_join() to clean up
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}