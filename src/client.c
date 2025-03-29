/*
 * client.c - An ftp client
 */

#include "client.h"

void response_treatment(int clientfd, request_t* request)
{
    int request_type = request->type;
    switch(request_type) {
        /* GET */
        case 0:
            get_response(clientfd, request);
            break;
        /* LS */
        case 1:
            /* ls command */
            ls_response(clientfd);
            break;
    }
}

void message_from_master(int clientfd, char** message) 
{
    rio_t rio;
    int message_size;

    Rio_readinitb(&rio, clientfd);
    Rio_readn(clientfd, &message_size, sizeof(message_size));

    message_size = ntohl(message_size);

    *message = malloc(message_size);
    if (!*message) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    Rio_readlineb(&rio, *message, message_size);      
}


void extract_slave_info(char* message, char* slave_ip, int* slave_port)
{
    char* token = strtok(message, "|");
    if (!token) return;
    strcpy(slave_ip, token);

    token = strtok(NULL, " ");
    if (!token) return;
    *slave_port = atoi(token);
}

int main(int argc, char **argv)
{
    int clientfd;
    int port = 2168; /* Default port used to communicate with the Master */
    char* host; /* ip addr */
    char input[MAXLINE];
    rio_t rio;

    host = argv[1];
    clientfd = Open_clientfd(host, port); // Client connects to the master
    
    if (clientfd < 0) {
        printf("server not available, please try again later\n");
        exit(0);
    }
    printf("Client connected to server\n");
    
    Rio_readinitb(&rio, clientfd);

    char* message = NULL;
    message_from_master(clientfd, &message); // Reading from master

    char slave_ip[21];
    extract_slave_info(message, slave_ip, &port); // Extracting slave's IP and Port from master's message
    printf("%s\n", message);
    Close(clientfd); // Close the connection between the master and the client
    
    clientfd = Open_clientfd(slave_ip, port); // Open a connection between the server and the client

    while (1) {

        request_t* request = malloc(sizeof(request_t));

        char log_path[256];
        snprintf(log_path, sizeof(log_path), "clientside/.log/.log");

        /* Checking if a crash has occured before connecting to the server */
        if (access(log_path, F_OK) == 0) {
            uint32_t total_bytes_sent, file_size;

            /* Retrieve the request information from the log file and resume the transfer from the last saved position */
            if (parse_resume_log(log_path, request, &total_bytes_sent, &file_size) == 0) {
                request->total_bytes_sent = total_bytes_sent;
                printf("Resuming transfer of %s from byte %u\n", request->filename, total_bytes_sent);
                resume_file_transfer(clientfd, request);
            }
        }

        printf("ᓚᘏᗢ ");
        if (Fgets(input, MAXLINE, stdin) == NULL) {
            break; //(Ctrl+D) 
        }
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') { // If there is input and it ends with a newline character, remove the newline before processing
            input[len - 1] = '\0';
        }
        if (strcmp(input, "bye") == 0) { // Close the connection and exit the program
            Close(clientfd);
            break;
        }
        if (strcmp(input, "") == 0) { // If no input was provided, skip further processing
            continue;
        }

        /* Process the input */
        request = fill_request_struct(input, request);
        
        if (request){
            char* serialized_request = serialize_request(request);
    
            send_request(clientfd, serialized_request);

            response_treatment(clientfd, request);

            free(request);
        }
        
    }
    exit(0);
}