#ifndef __GET_H__
#define __GET_H__

#include "request.h"

#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_NAME_LEN 256
#define BLOCK_SIZE 64*1024 /* 64Kb - used for sending files */

/**
 * Manages the file pathname, creating a new file to write received data.
 *
 * @param request A pointer to the request structure containing information about the file.
 * @return File descriptor of the created file, or -1 if there is an error.
 */
int manage_pathname(request_t* request);

/**
 * Creates a file by receiving data from the client and writing it to the file descriptor.
 *
 * @param clientfd The file descriptor of the connection between the server and the client.
 * @param fd The file descriptor of the file to write data to.
 * @param file_bytes_sent Number of bytes sent by the server (higher than 0 if there was crash).
 * @param file_size The total size of the file to be received.
 * @param request A pointer to the request structure.
 */
void create_file(int clientfd, int fd, uint32_t file_bytes_sent, uint32_t file_size, request_t* request);

/**
 * Prints the file size in a human-readable format (bytes, KB, MB, GB).
 *
 * @param file_size The size of the file in bytes.
 * @param display_size A pointer to the value of the file size to be displayed.
 * @param unit A pointer to the unit of measurement (e.g., "KB", "MB", "GB").
 */
void print_display_size(uint32_t file_size, double* display_size, char** unit);

/**
 * Handles the response from the client, managing file reception.
 *
 * @param clientfd The file descriptor of the connection between the server and the client.
 * @param request A pointer to the request structure.
 */
void get_response(int clientfd, request_t* request);

/**
 *  Reads and prints an error message sent by the server.
 *
 * @param clientfd The file descriptor of the connection between the server and the client.
 */
void read_error(int clientfd);

/**
 * Constructs the pathname for a temporary file to store part of the received file.
 *
 * @param request A pointer to the request structure.
 * @return A pointer to the constructed pathname string, or NULL if memory allocation fails.
 */
char* construct_pathname(request_t* request);

/**
 * Parses a resume log file to retrieve the file size and the total bytes sent.
 *
 * @param log_path The path to the resume log file.
 * @param request A pointer to the request structure.
 * @param total_bytes_sent Pointer to store the total bytes sent from the log.
 * @param file_size Pointer to store the file size from the log.
 * @return 0 on success, -1 on failure.
 */
int parse_resume_log(const char* log_path, request_t* request, uint32_t* total_bytes_sent, uint32_t* file_size);

/**
 * Logs the file transfer details into a temporary log file and renames it to the final log.
 *
 * @param filename The name of the file being transferred.
 * @param file_size The size of the file being transferred.
 * @param total_bytes_sent The total number of bytes recieved so far.
 */
void log_file_transfer(char* filename, uint32_t file_size, uint32_t total_bytes_sent);

/**
 * Resumes a file transfer by sending the serialized request and handling the response.
 *
 * @param clientfd The file descriptor of the connection between the server and the client.
 * @param request A pointer to the request structure.
 */
void resume_file_transfer(int clientfd, request_t* request);

#endif