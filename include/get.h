#ifndef __GET_H__
#define __GET_H__

#include "request.h"

#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_NAME_LEN 256
#define BLOCK_SIZE 64*1024 /* 64Kb - used for sending files */

int manage_pathname(request_t* request);
void create_file(int clientfd, int fd, uint32_t file_bytes_sent, uint32_t file_size, request_t* request);
void print_display_size(uint32_t file_size, double* display_size, char** unit);
void get_response(int clientfd, request_t* request);
void read_error(int clientfd);
char* construct_pathname(request_t* request);
int parse_resume_log(const char* log_path, request_t* request, uint32_t* total_bytes_sent, uint32_t* file_size);
void log_file_transfer(char* filename, uint32_t file_size, uint32_t total_bytes_sent);
void resume_file_transfer(int clientfd, request_t* request);

#endif
