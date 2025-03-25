#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"
#include "handlers.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#define FILENAME_MAXSIZE 50 /* filename's max size */
#define MAX_NAME_LEN 256
#define BLOCK_SIZE 64*1024 /* 64Kb - used for sending files */

typedef struct request_t {
    int type; /* a preciser dans le compte rendu */
    char filename[FILENAME_MAXSIZE];
    size_t filename_size; /* file size */
    uint32_t total_bytes_sent;
} request_t;

request_t* fill_request_struct(char* input, request_t* req);
char* serialize_request(request_t* req);
void send_request(int clientfd, char* request);
void read_error(int clientfd);
int manage_pathname(request_t* request);
void create_file(int clientfd, int fd, uint32_t file_bytes_sent, uint32_t file_size, request_t* request);
void get_request(int clientfd, request_t* request);
void manage_commands(int clientfd, request_t* request);
char* construct_pathname(request_t* request);
void log_file_transfer(char* filename, uint32_t file_size, uint32_t total_bytes_sent);
void resume_file_transfer(int clientfd, request_t* request, uint32_t resume_offset, uint32_t file_size);
void read_from_master(int clientfd);

#endif