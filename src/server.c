/*
 * server.c - An ftp server
 */

#include "server.h"

pid_t pool[NB_PROC];
int connection_closed = 0;

request_t* decode_request(char* serialized_request)
{
    request_t* req = malloc(sizeof(request_t));

    if (!req) return NULL;

    /* serialized_request looks like -> 0|12|exemple.txt|0 */
    /* strtok splits a string into tokens based on the specified delimiter */
    char* token = strtok(serialized_request, "|");
    req->type = atoi(token);
    
    token = strtok(NULL, "|");
    req->filename_size = atoi(token);

    token = strtok(NULL, "|");
    strncpy(req->filename, token, FILENAME_MAXSIZE);

    token = strtok(NULL, " \n");
    req->total_bytes_sent = atoi(token);

    return req;
}

void process_request(int connfd)
{
    ssize_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    /* While the connection is open, the server keeps reading from the socket */
    while (!connection_closed && (n = rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        printf("server received %u bytes\n", (unsigned int)n);

        request_t* req = decode_request(buf);
        printf("%s | %ld | %s | %u\n", req->type == 0 ? "GET" : "NOTGET", req->filename_size, req->filename, req->total_bytes_sent);

        manage_requests(connfd, req);
    }
}

void manage_requests(int connfd, request_t* req)
{
    switch (req->type) {
        case 0: /* GET */
            file_manager(connfd, req);
            break;
    }
}

/* Sends an error message following the specified protocol */
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
        default:
            send_error(connfd, 400, "Access error");
    }
}

void file_manager(int connfd, request_t* req)
{
    int pathname_size = strlen(req->filename) + strlen("serverside/") + 1;
    char* pathname = (char*)malloc(pathname_size);
    if (pathname == NULL) {
        perror("malloc");
        return;
    }

    snprintf(pathname, pathname_size, "serverside/%s", req->filename);
    
    if (access(pathname, F_OK | R_OK) == 0) {  // Check if the file exists and is readable
        int fd = open(pathname, O_RDONLY);
        if (fd >= 0) { 
            /* If a crash occurred previously (total_bytes_sent > 0) */
            /* Move the file pointer to the position indicated by total_bytes_sent to resume sending the file. */
            if (lseek(fd, req->total_bytes_sent, SEEK_SET) == -1) {
                perror("lseek failed");
                return;
            }
    
            struct stat metadata;
            stat(pathname, &metadata); // Get the file metadata
    
            rio_t rio;
            Rio_readinitb(&rio, fd);
    
            /* According to the protocol, send the file size to the client first */
            uint32_t size_net = htonl((uint32_t)metadata.st_size);
            Rio_writen(connfd, &size_net, sizeof(size_net));
    
            int n;
            char file_buffer[BLOCK_SIZE];
            while((n = read(fd, file_buffer, BLOCK_SIZE)) > 0) {  // Read file in blocks
                uint32_t n_net = htonl(n);
                
                /* SIGPIPE handling is included here because SIGPIPE is being ignored using SIG_IGN */
                /*  Send the number of bytes read to the client */
                if (rio_writen(connfd, &n_net, sizeof(n_net)) != (ssize_t)sizeof(n_net)) {
                    /* If the written data size doesn't match the expected size */
                    /* A crash has occurred */
                    Close(connfd);
                    connection_closed = 1;
                    break;
                }
    
                /* Send the actual file data to the client */
                if (rio_writen(connfd, file_buffer, n) != (ssize_t)n) {
                    /* If the written data size doesn't match the expected size */
                    /* A crash has occurred */
                    Close(connfd);
                    connection_closed = 1;
                    break;
                }
            }

            close(fd);

        } else {
            perror("open");
        }
    } else {
        // If the file doesn't exist or isn't readable
        manage_errors(connfd, errno);
    }
    free(pathname);
}



int main(int argc, char **argv)
{
    Signal(SIGINT, handler_sigint);
    Signal(SIGPIPE, SIG_IGN); /* Ignoring SIGPIPE and handling it in file_manager */
    
    pid_t pid;
    int port = 2169; /* Default port */
    int listenfd = Open_listenfd(port);

    /* Creation of the process pool */
    for (int i = 0; i < NB_PROC; i++ ) {
        pid  = Fork();
        if (pid == 0) break;
        pool[i] = pid;
    }
    
    if (pid == 0) {  
        Signal(SIGINT, SIG_DFL); /* Restore the default behaviour for the SIGINT signal. */

        int connfd;
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        char client_ip_string[INET_ADDRSTRLEN];
        char client_hostname[MAX_NAME_LEN];
    
        clientlen = (socklen_t)sizeof(clientaddr);
            
        while (1) {
            /* Waiting for connection */
            while ((connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0);  
            
            /* Gets connection */
            connection_closed = 0;      

            /* Determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* Determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                    INET_ADDRSTRLEN);
            
            printf("server connected to %s (%s) | client info: %u\n", client_hostname,
                client_ip_string, clientaddr.sin_addr.s_addr);

            /* Process the request */
            process_request(connfd);
            
            if (!connection_closed) Close(connfd);
            printf("Connection to (%s) closed\n", client_ip_string);
        }
    } else {
        pause();
    }
}