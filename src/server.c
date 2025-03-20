/*
 * server.c - An ftp server
 */

#include "server.h"

#define MAX_NAME_LEN 256

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

void request_routage(request_t* req)
{
    switch (req->type) {
        case 0: /* GET */
            send_file(req);
            break;
    }
}

void send_file(request_t* req)
{
    // Construct the pathname
    int pathname_size = strlen(req->filename) + strlen("./serverside/") + 1;
    char* pathname = (char*)malloc(pathname_size);
    if (pathname == NULL) {
        perror("malloc");
        return;
    }
    
    snprintf(pathname, pathname_size, "./serverside/%s", req->filename);
    
    // Resolve the real path
    char real[MAX_NAME_LEN];
    realpath(pathname, real);
    printf("Resolved path: %s\n", real);
    if (real == NULL) {
        perror("realpath");
        free(pathname);
        return;
    }
    
    
    if (access(real, F_OK | R_OK) != -1) {
        // File exists and is readable
        int fd = open(real, O_RDONLY);
        if (fd >= 0) {
            printf("wemadeit\n");
            
            struct stat metadata;
            if (stat(real, &metadata) < 0) {
                perror("stat");
                close(fd);
                free(pathname);
                return;
            }
            
            rio_t rio;
            Rio_readinitb(&rio, fd);
            // Continue with file operations...
            
            // Don't forget to close the file when done
            // close(fd);
        } else {
            perror("open");
        }
    } else {
        // File doesn't exist or isn't readable
        if (errno == ENOENT) {
            printf("File does not exist: %s\n", real);
        } else if (errno == EACCES) {
            printf("Permission denied: %s\n", real);
        } else {
            perror("access");
        }
    }
    
    free(pathname);  // Don't forget to free allocated memory
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
        request_routage(req);
    }
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
        //printf("%d\n", pool[i]);
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