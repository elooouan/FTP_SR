#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"
#include "handlers.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NB_PROC 1 /* number of processes (children) in process pool */
#define FILENAME_MAXSIZE 50 /* filename's max size */
#define MAX_NAME_LEN 256
#define BLOCK_SIZE 64*1024 /* 64Kb - used for sending files */

extern pid_t pool[NB_PROC];

typedef struct request_t {
    int type; /* a preciser dans le compte rendu */
    char filename[FILENAME_MAXSIZE];
    size_t filename_size; /* file size */
    uint32_t total_bytes_sent;
} request_t;

typedef struct response_t {    
    int code; /* return code. Success = 0, Error = 1 */
} response_t;

request_t* decode_request(char* serialized_request);
void process_request(int connfd);
void manage_requests(int connfd, request_t* req);
void send_error(int connfd, int error_code, char* error_message);
void manage_errors(int connfd, int error_code);
void file_manager(int connfd, request_t* req);


#endif