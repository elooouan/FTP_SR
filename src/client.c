/*
 * client.c - An ftp client
 */

#include "client.h"

request_t* fill_request_struct(char* input)
{
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

char* serialize_request(request_t* req)
{
    size_t message_size = sizeof(req->type) + sizeof(req->filename_size) + strlen(req->filename) + 2 + 1; /* two '|' and one '/0' */
    char* message = (char*)malloc(message_size);

    if (!message) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    snprintf(message, message_size, "%d|%zu|%s", req->type, req->filename_size, req->filename);
    printf("%s\n", message);
    return message;
}

void send_request(int clientfd, char* request)
{
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    Rio_writen(clientfd, request, strlen(request));
    
    // uint32_t file_size;
    // if (Rio_readn(clientfd, &file_size, sizeof(file_size)) > 0) {
    //     file_size = ntohl(file_size);
    //     printf("File size: %u bytes\n", file_size);
    // }
}

void read_error(int clientfd)
{
    int error_code;
    Rio_readn(clientfd, &error_code, sizeof(int));
    error_code = ntohl(error_code);

    int msg_len;
    Rio_readn(clientfd, &msg_len, sizeof(msg_len));
    msg_len = ntohl(msg_len);

    char *message = malloc(msg_len + 1);
    if (message == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    Rio_readn(clientfd, message, msg_len);

    message[msg_len] = '\0';

    printf("Error %d: %s\n", error_code, message);

    free(message);
}


int main(int argc, char **argv)
{
    int clientfd;
    int port = 2121; /* by default */
    char *host;
    char input[MAXLINE];
    rio_t rio;
    
    host = argv[1];
    clientfd = Open_clientfd(host, port);
    
    if (clientfd < 0) {
        printf("server not available rn gng\n");
        exit(0);
    }
    printf("client connected to server OS\n");
    
    Rio_readinitb(&rio, clientfd);

    while (Fgets(input, MAXLINE, stdin) != NULL) {
        request_t* request = fill_request_struct(input);
        
        if(request){
            char* serialized_request = serialize_request(request);
            send_request(clientfd, serialized_request);

            uint32_t file_size, n, file_bytes_sent;
            if ((n = Rio_readn(clientfd, &file_size, sizeof(file_size)))> 0) {
                file_size = ntohl(file_size);

                /* error code */
                if (file_size == -1) {
                    read_error(clientfd);
                } else {
                    printf("File size: %u bytes\n", file_size);
                    
                    int pathname_size = strlen(request->filename) + strlen("clientside/") + 1;
                    char* pathname = (char*)malloc(pathname_size);
                    if (pathname == NULL) {
                        perror("malloc");
                        return -1;
                    }
                    
                    snprintf(pathname, pathname_size, "clientside/%s", request->filename);
                    pathname[strlen(pathname) - 1] = '\0'; /* due to client-side *enter* */ 

                    int fd = open(pathname, O_WRONLY | O_CREAT, 0666); /* read and write permissions */
                    if (fd < 0) {
                        perror("opening/creating file failed");
                        return -1;
                    }

                    int total_bytes_sent = 0;
                    clock_t begin = clock();
                    while ((n = Rio_readn(clientfd, &file_bytes_sent, sizeof(file_bytes_sent)) > 0)) {
                        file_bytes_sent = ntohl(file_bytes_sent);
                        total_bytes_sent += file_bytes_sent;
                        char file_buffer[file_bytes_sent];

                        Rio_readn(clientfd, file_buffer, file_bytes_sent);

                        write(fd, file_buffer, file_bytes_sent);

                        if (total_bytes_sent == file_size) {
                            printf("Transfer successfully complete.\n");
                            break;
                        }
                    }

                    clock_t end = clock();
                    int delay = (end - begin) / CLOCKS_PER_SEC;

                    printf("%d bytes received in %d seconds\n", total_bytes_sent, delay);
                    close(fd);
                }
            }

            free(request);
        }

        
    }

    Close(clientfd);
    exit(0);
}
