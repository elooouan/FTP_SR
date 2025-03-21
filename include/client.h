#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"
#include "handlers.h"
#include "string.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define NB_PROC 3 /* number of processes (children) in process pool */
#define FILENAME_MAXSIZE 50 /* filename's max size */
#define MAX_NAME_LEN 256
#define BLOCK_SIZE 1024 /* used for sending files */

extern pid_t pool[NB_PROC];

typedef struct request_t {
    int type; /* a preciser dans le compte rendu */
    char filename[FILENAME_MAXSIZE];
    size_t filename_size; /* file size */

} request_t;

request_t* fill_request_struct(char* input);
char* serialize_request(request_t* req);
void send_request(int clientfd, char* request);
void read_error(int clientfd);

#endif