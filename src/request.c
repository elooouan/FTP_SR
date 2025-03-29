#include "request.h"

int get_request_type(char* command)
{
    if (strcmp(command, "get") == 0) return 0;
    else if (strcmp(command, "ls") == 0) return 1;
    else if (strcmp(command, "put") == 0) return 2;
    else if (strcmp(command, "rm") == 0) return 3;
    return -1;
}

request_t* fill_request_struct(char* input, request_t* req)
{
    if (!req) return NULL;

    /* GET request looks like -> get abcdefg.txt */
    /* strtok splits a string into tokens based on the specified delimiter */
    char* token = strtok(input, " ");
    if (!token) {
        free(req);
        return NULL;
    }

    int type = get_request_type(token);
    if (type < 0) {
        fprintf(stderr, "%s: command does not exist\n", token);
        free(req);
        return NULL;   
    }
    req->type = type;

    /* ls command */
    /* Ignores any input that follows the "ls" command */
    if (type == 1) {
        strncpy(req->filename, "empty", FILENAME_MAXSIZE);
        req->filename_size = 0;
        req->total_bytes_sent = 0;
        return req;
    }

    token = strtok(NULL, " ");
    if (!token || strlen(token) == 0) {
        fprintf(stderr, "please specify a filename\n");
        free(req);
        return NULL;
    }

    strncpy(req->filename, token, FILENAME_MAXSIZE);
    req->filename_size = strlen(req->filename);
    req->total_bytes_sent = 0;

    return req;
}

char* serialize_request(request_t* req)
{
    size_t message_size = sizeof(req->type) + sizeof(req->filename_size) + strlen(req->filename) + sizeof(req->total_bytes_sent) + 2 + 1;
    char* message = malloc(message_size);
    if (!message) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    snprintf(message, message_size, "%d|%zu|%s|%u\n", req->type, req->filename_size, req->filename, req->total_bytes_sent);
    return message;
}

void send_request(int clientfd, char* request)
{
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    Rio_writen(clientfd, request, strlen(request));
}