#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"
#include "handlers.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NB_PROC 3 /* number of processes (children) in process pool */
#define FILENAME_MAXSIZE 20 /* filename's max size */

extern pid_t pool[NB_PROC];

typedef struct request_t {
    int type; /* a preciser dans le compte rendu */
    char filename[FILENAME_MAXSIZE];
    size_t filename_size; /* file size */

} request_t;

typedef struct response_t {    
    int code; /* return code. Success = 0, Error = 1 */
} response_t;

request_t* decode_request(char* serialized_request);
void process_request(int connfd);
void send_file(request_t* req);


#endif