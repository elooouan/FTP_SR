/*
 * client.c - An ftp client
 */
#include "csapp.h"
#include "server.h"
#include "string.h"

request_t* fill_request_struct(char* input) {
    request_t* req = malloc(sizeof(request_t));
    
    if (!req) return NULL;

    char* token = strtok(input, " ");
    if (!token) {
        free(req);
        return NULL;
    }

    if (strcmp(token, "get") == 0) {
        req->type = 0; /* 0 -> GET */
    } else {
        free(req);
        fprintf(stderr, "%s: command does not exist\n", token);
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token == NULL) {
        free(req);
        return NULL;
    }

    strncpy(req->filename, token, FILENAME_MAXSIZE);
    req->filename[FILENAME_MAXSIZE - 1] = '\0';

    // req->file_size = sizeof(name);

    return req;
}

char* serialize_request(request_t* req) {
    size_t message_size = sizeof(req->type) + sizeof(req->filename_size) + strlen(req->filename) + 2 + 1; /* two '|' and one '/0' */
    char* message = (char*)malloc(message_size);

    if (!message) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    snprintf(message, message_size, "%d|%zu|%s", req->type, req->filename_size, req->filename);

    return message;
}

void send_request(int clientfd, char* request) {
    rio_t rio;

    Rio_readinitb(&rio, clientfd);
    
    Rio_writen(clientfd, request, strlen(request));
    
}

int main(int argc, char **argv)
{
    int clientfd;
    int port = 2121; /* by default */
    char *host;
    char input[MAXLINE];
    // rio_t rio;
    
    host = argv[1];

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
    */
    printf("client connected to server OS\n"); 
    
    while (Fgets(input, MAXLINE, stdin) != NULL) {
        request_t* request = fill_request_struct(input);
        if(request){
            char* serialized_request = serialize_request(request);
            printf("reqqq : %s\n", serialized_request);
            send_request(clientfd, serialized_request);
            free(request);
        }
    }

    // Rio_readinitb(&rio, clientfd);

    // while (Fgets(buf, MAXLINE, stdin) != NULL) {
    //     Rio_writen(clientfd, buf, strlen(buf));
    //     if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
    //         Fputs(buf, stdout);
    //     } else { /* the server has prematurely closed the connection */
    //         break;
    //     }
    // }
    Close(clientfd);
    exit(0);
}
