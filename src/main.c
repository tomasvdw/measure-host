/* main.c
 *  
 *  A tcp listening server 
 *  Uses request.c to handle incoming requests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <sys/socket.h>
#include <netinet/ip.h>

#include "request.h"

#define PORT             6423

// listen queue
#define QUEUE_SIZE       25

#define READ_CHUNK_SIZE  1024
#define MAX_REQUEST_SIZE 32 * 1024


// file descriptor set for select()
static fd_set  fdmask_main;    // sockets active
static int     fdmax;          // highest socket#
static int     fd_listen;      // listening socket


// input buffers for the clients
struct client
{
    char *buffer;
    int   len;
};
// we'll keep an array of fdmax of these
static struct client *clients; 

// Sets up the listening socket and starts listening
static void server_listen()
{
    struct sockaddr_in serveraddr;

    // setup socket
    fd_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_listen == -1) {
        perror("socket() failed");
        exit(1);
    }

    // bind the socket to the port
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)PORT);
    if (bind(fd_listen, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind() failed");
        exit(1);
    }

    if (listen(fd_listen, QUEUE_SIZE) < 0) 
    {
        perror("listen() failed");
        exit(1);
    }

    // add the listener to the select() mask
    FD_ZERO(&fdmask_main);
    FD_SET(fd_listen, &fdmask_main);
    fdmax = fd_listen;

}

// accept new connection
static void server_accept()
{
    struct sockaddr_in clientaddr;
    unsigned size = sizeof (clientaddr);
    int incoming_fd = accept(fd_listen, 
            (struct sockaddr *) &clientaddr, &size);

    if (incoming_fd < 0) {
        perror("accept() failed");
        // probably not going to work
        // but let's just retry anyway
        return; 
    }

    printf("Incoming connection @ %d\n", incoming_fd);

    // include this in the select() mask
    FD_SET(incoming_fd, &fdmask_main);
    if (incoming_fd > fdmax)
    {
        fdmax = incoming_fd;
        clients = realloc(clients, sizeof(struct client) * (fdmax+1));
    }
    clients[incoming_fd].len = 0;
}

void server_closesock(int sock)
{
    printf("Closing %d\n", sock);
    free(clients[sock].buffer);
    close(sock);
    FD_CLR(sock, &fdmask_main);
}

// reads on the given sock and stores it in the client buffer
// returns 0 if the connection was closed
// or the amout of data read
int server_read(int sock)
{
    //printf("Incoming data @%d\n", sock);

    // make space in client buffer
    int needed_space = clients[sock].len + READ_CHUNK_SIZE + 1;
    if (clients[sock].len == 0)
        clients[sock].buffer = malloc(needed_space);
    else
        clients[sock].buffer = realloc(clients[sock].buffer, needed_space);


    int r = read( sock, 
            clients[sock].buffer + clients[sock].len, 
            READ_CHUNK_SIZE);


    if (r <= 0
        || r + clients[sock].len > MAX_REQUEST_SIZE)
    {
        // error / EOF / max size reached: handle the same
        server_closesock(sock);
        return 0;
    }
    else
    {
        // read and store in client[] buffer
        clients[sock].len += r;
        clients[sock].buffer[clients[sock].len] = '\0';
        return r;
    }
}

// checks if a full request has been read for sock
// and processes and responds if possible
void server_tryprocess(int sock)
{
    int end = request_findend(clients[sock].buffer);
    if (end == -1)
        return; // not yet a complete request

    char *result = request_process(clients[sock].buffer, end);

    if (result == NULL)
    {
        // error, close connection
        server_closesock(sock);
        return;
    }
    // success.
    // something to send?
    if (*result != '\0') 
    {
        int s = write(sock, result, strlen(result));
        if (s < 0)
        {
            perror("write() failed");
            server_closesock(sock);
            return;
        }
    }
    free(result);

    // move data after this request to the start of the buffer
    memmove(clients[sock].buffer, clients[sock].buffer+end, 
            clients[sock].len - end + 1);
    clients[sock].len -= end;

}


// runs the tcp-server
static void server_run()
{
    server_listen();

    // we'll setup a client buffer for each (possible) fd
    clients = calloc(fdmax+1, sizeof(struct client));

    // main loop
    for(;;) {

        // sockets being checked
        fd_set fdmask_check = fdmask_main;

        // wait for data/connection
        if (select(fdmax+1, &fdmask_check, NULL, NULL, NULL)  == -1) {
            perror("select() failed");
            exit(1);
        }

        for(int i =0; i < fdmax+1; i++) 
        {
            if (!FD_ISSET(i, &fdmask_check))
                continue;

            if (i == fd_listen)
            {
                // incoming connection
                server_accept();
            }
            else
            {
                // read data and process if possible
                if (server_read(i))
                    server_tryprocess(i);
            }
        }
    }
}


int main(void) {

    puts("Starting service");

    server_run();

    return 0;
}


