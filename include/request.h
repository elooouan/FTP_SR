#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "csapp.h"

#include <sys/types.h>
#include <time.h>
#include <stdlib.h>

#define FILENAME_MAXSIZE 50 /* filename's max size */

typedef struct request_t {
    int type; /* a preciser dans le compte rendu */
    char filename[FILENAME_MAXSIZE];
    size_t filename_size; /* file size */
    uint32_t total_bytes_sent;
} request_t;

int request_type(char* command);
request_t* fill_request_struct(char* input, request_t* req);
char* serialize_request(request_t* req);
void send_request(int clientfd, char* request);

#endif