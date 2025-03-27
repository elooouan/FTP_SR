/*
 * server.c - An ftp server
 */

#include "server.h"

pid_t pool[NB_PROC];
int connection_closed = 0;

request_t* decode_request(char* serialized_request)
{
    request_t* req = malloc(sizeof(request_t));

    if (!req) {
        fprintf(stderr, "Memory allocation failed for request_t\n");
        return NULL;
    }

    /* Tokenize using the correct delimiter: | */
    char* token = strtok(serialized_request, "|");

    /* Get the request type */
    req->type = atoi(token);
    if (req->type < 0) {
        fprintf(stderr, "Invalid request type\n");
        free(req);
        return NULL;
    }

    /* Get the filename size */
    token = strtok(NULL, "|");
    req->filename_size = atoi(token);
    if (req->filename_size < 0) {
        fprintf(stderr, "Invalid filename size\n");
        free(req);
        return NULL;
    }

    /* Get the filename */
    token = strtok(NULL, "|");
    if (token) {
        strncpy(req->filename, token, FILENAME_MAXSIZE);
    } else {
        strncpy(req->filename, "empty", 6);
    }

    /* Get the total bytes sent */
    token = strtok(NULL, "|");
    if (token) {
        req->total_bytes_sent = atoi(token);
    } else {
        req->total_bytes_sent = 0;
        printf("total_bytes_sent is missing or invalid, setting to 0\n");
    }

    return req;
}

void process_request(int connfd)
{
    ssize_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while (!connection_closed && (n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        request_t* req = decode_request(buf);
        printf("%s | %ld | %s | %u\n", req->type == 0 ? "GET" : (req->type == 1 ? "LS" : "NOTGET"), req->filename_size, req->filename, req->total_bytes_sent);
        manage_requests(connfd, req);
    }
}

void display_files(int connfd, request_t* req)
{
    FILE *fp = popen("ls serverside", "r");
    if (!fp) {
        perror("popen failed");
        return;
    }

    char buffer[4096]; 
    int bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes_read] = '\0';
    pclose(fp);

    int message_size = htonl(bytes_read);

    // Send size
    Rio_writen(connfd, &message_size, sizeof(message_size));

    // Send ls output
    Rio_writen(connfd, buffer, bytes_read);
}


void manage_requests(int connfd, request_t* req)
{
    switch (req->type) {
        case 0: /* get */
            file_manager(connfd, req);
            break;
        case 1: /* ls */
            display_files(connfd, req);
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

    if (access(pathname, F_OK | R_OK) == 0) {
        int fd = open(pathname, O_RDONLY);
        if (fd >= 0) {
            if (lseek(fd, req->total_bytes_sent, SEEK_SET) == -1) {
                perror("lseek failed");
                return;
            }

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

                if (rio_writen(connfd, &n_net, sizeof(n_net)) != (ssize_t)sizeof(n_net)) { 
                    Close(connfd);
                    break;
                }

                if (rio_writen(connfd, file_buffer, n) != (ssize_t)n) {
                    Close(connfd);
                    connection_closed = 1;
                    break;
                }
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

/* sends disconnection status to master */
void send_disconnection_status(int serverfd, char* server_ip_string)
{
    int ip_size = strlen(server_ip_string);
    int status_size = strlen("DISCONNECTED|") + ip_size + 1;
    char* status = malloc(status_size);

    snprintf(status, status_size, "DISCONNECTED|%s", server_ip_string);
    printf("notifying master: %s\n", status);

    int status_size_net = htonl(status_size);
    
    Rio_writen(serverfd, &status_size_net, sizeof(int));
    Rio_writen(serverfd, status, status_size);

    Close(serverfd);
    free(status);
}

char* get_server_ip(int serverfd)
{
    int server_ip_size;

    Rio_readn(serverfd, &server_ip_size, sizeof(server_ip_size));

    server_ip_size = ntohl(server_ip_size);

    char* server_ip = malloc(server_ip_size);
    if (!server_ip) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    Rio_readn(serverfd, server_ip, server_ip_size);  

    return server_ip;
}

int main(int argc, char **argv)
{
    Signal(SIGINT, handler_sigint);
    Signal(SIGPIPE, SIG_IGN);
    
    pid_t pid;
    int port = 2122; /* slave port (you can choose any by default != 2121) */
    int listenfd = Open_listenfd(port);

    /* master port */
    char master_IP[INET_ADDRSTRLEN] = "152.77.82.179"; /* Modify as needed*/
    int master_port = 2120;

    /* creation of the process pool */
    for (int i = 0; i < NB_PROC; i++ ) {
        pid  = Fork();
        if (pid == 0) break;
        pool[i] = pid;
    }

    if (pid == 0) {  
        Signal(SIGINT, SIG_DFL);
        
        int connfd;
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        char client_ip_string[INET_ADDRSTRLEN];
        char client_hostname[MAX_NAME_LEN];
        
        clientlen = (socklen_t)sizeof(clientaddr);

        while (1) {
            /* waiting for client connection */
            while ((connfd = accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0);
            
            int serverfd = Open_clientfd(master_IP, master_port);  
            char server_IP[INET_ADDRSTRLEN];
            strcpy(server_IP, get_server_ip(serverfd));

            /* GETS CONNECTION */
            connection_closed = 0;

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
            
            fprintf(stderr,"server connected to %s (%s)\n", client_hostname, client_ip_string);


            /* manage request */
            process_request(connfd);


            /* CLOSES CONNECTION */
            if (!connection_closed) Close(connfd);
            printf("Connection to (%s) closed\n", client_ip_string);
            
            /* tell master the client disconnected */
            send_disconnection_status(serverfd, server_IP);
        }
    } else {
        pause();
    }
}