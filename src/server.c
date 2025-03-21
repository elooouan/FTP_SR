/*
 * server.c - An ftp server
 */

#include "server.h"

pid_t pool[NB_PROC];

request_t* decode_request(char* serialized_request)
{
    request_t* req = malloc(sizeof(request_t));

    if (!req) return NULL;

    /* might need error handling if coonection/pipe crashes */
    char* token = strtok(serialized_request, "|");
    req->type = atoi(token);
    
    token = strtok(NULL, "|");
    req->filename_size = atoi(token);

    token = strtok(NULL, " ");
    strncpy(req->filename, token, FILENAME_MAXSIZE);

    return req;
}

void process_request(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        request_t* req = decode_request(buf);
        printf("%s | %ld | %s\n", req->type == 0 ? "GET" : "NOTGET", req->filename_size, req->filename);
        request_routage(connfd, req);
    }
}

void request_routage(int connfd, request_t* req)
{
    switch (req->type) {
        case 0: /* GET */
            file_manager(connfd, req);
            break;
    }
}

void send_error(int connfd, int error_code, char* error_message)
{
    uint32_t error_indicator = htonl(0xFFFFFFFF); /* to indicate an error -> warning */
    Rio_writen(connfd, &error_indicator, sizeof(error_indicator));
    
    error_code = htonl(error_code);
    Rio_writen(connfd, &error_code, sizeof(error_code));
    
    uint32_t msg_len = strlen(error_message);
    uint32_t msg_len_net = htonl(msg_len);
    Rio_writen(connfd, &msg_len_net, sizeof(msg_len_net));
    
    Rio_writen(connfd, error_message, msg_len);
}

void manage_errors(int connfd, int error_code)
{
    switch (error_code) {
        case ENOENT:
            send_error(connfd, 404, "File does not exist");
            break;
        case EACCES:
            send_error(connfd, 403, "Permission denied");
            break;
        default:
            send_error(connfd, 400, "Access error");
    }
}

// void manage_pathname

void file_manager(int connfd, request_t* req)
{
    int pathname_size = strlen(req->filename) + strlen("serverside/") + 1;
    char* pathname = (char*)malloc(pathname_size);
    if (pathname == NULL) {
        perror("malloc");
        return;
    }
    
    snprintf(pathname, pathname_size, "serverside/%s", req->filename);
    pathname[strlen(pathname) - 1] = '\0'; /* due to client-side *enter* */

    if (access(pathname, F_OK | R_OK) == 0) {
        // File exists or Readable 
        int fd = open(pathname, O_RDONLY);
        if (fd >= 0) {
            
            struct stat metadata;
            stat(pathname, &metadata);
            
            rio_t rio;
            Rio_readinitb(&rio, fd);
            
            /* According to protocol -> send file size to client first */
            uint32_t size_net = htonl((uint32_t)metadata.st_size); /* For Endianness */
            Rio_writen(connfd, &size_net, sizeof(size_net));
            
            int n;
            char file_buffer[BLOCK_SIZE];
            while((n = read(fd, file_buffer, BLOCK_SIZE)) > 0) {
                uint32_t n_net = htonl(n);
                Rio_writen(connfd, &n_net, sizeof(n_net));
                Rio_writen(connfd, file_buffer, n);
            }

            /* close after sending file */ 
            close(fd);

        } else {
            perror("open");
        }
    } else {
        // File doesn't exist or isn't readable
        manage_errors(connfd, errno);
    }
    
    free(pathname);  // Don't forget to free allocated memory
}



int main(int argc, char **argv)
{
    pid_t pid;
    int port = 2121; /* default port */
    int listenfd = Open_listenfd(port);

    // Signal(SIGINT, handler_sigint);

    /* creation of the process pool */
    for (int i = 0; i < NB_PROC; i++ ) {
        pid  = Fork();
        if (pid == 0) break;
        pool[i] = pid;
    }

    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];


    if (pid == 0) {        
        clientlen = (socklen_t)sizeof(clientaddr);
        

        while (1) {
            while ((connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0);

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                    INET_ADDRSTRLEN);
            
            printf("server connected to %s (%s)\n", client_hostname,
                client_ip_string);

            /* traitement */
            process_request(connfd);
            Close(connfd);
        }
    } else {
        pause();
    }
}