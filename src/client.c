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
    }
}

int main(int argc, char **argv)
{
    int clientfd;
    int port = 2121; /* Default port used by the server*/
    char* host;
    char input[MAXLINE];
    rio_t rio;

    host = argv[1];
    clientfd = Open_clientfd(host, port);
    
    if (clientfd < 0) {
        printf("server not available right now, please try again later.\n");
        exit(0);
    }
    printf("Client connected to server\n");
    
    Rio_readinitb(&rio, clientfd);

    while (1) {

        request_t* request = malloc(sizeof(request_t));

        /* Checking if a crash has occured before connecting to the server */
        char log_path[256];
        snprintf(log_path, sizeof(log_path), "clientside/.log/.log");

        if (access(log_path, F_OK) == 0) { /* If there was a crash*/
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
        if (len > 0 && input[len - 1] == '\n') { /* If there is input and it ends with a newline character, remove the newline before processing */
            input[len - 1] = '\0';
        }

        if (strcmp(input, "bye") == 0) { /* Close the connection and exit the program */
            Close(clientfd);
            break;
        }

        if (strcmp(input, "") == 0) { /* If no input was provided, skip further processing */
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
