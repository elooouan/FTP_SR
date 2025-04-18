#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"
#include "handlers_server.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NB_PROC 2 /* number of processes (children) in process pool */
#define FILENAME_MAXSIZE 50 /* filename's max size */
#define MAX_NAME_LEN 256
#define BLOCK_SIZE 64*1024 /* 64Kb - used for sending files */

extern pid_t pool[NB_PROC];

/** 
 * 
 * Request type:
 * 
 * 0 -> GET
 * 1 -> LS
 * 2 -> PUT (not implemented)
 * 3 -> RM (not implemented)
 * -1 -> NA
 * 
 */
typedef struct request_t {
    int type; // request type 
    char filename[FILENAME_MAXSIZE]; // file name
    size_t filename_size; // file size
    uint32_t total_bytes_sent; // number of bytes already recieved
} request_t;

/**
 * Decodes a serialized request received by the client.
 *
 * This function converts a serialized string into a `request_t` structure
 * containing the details of the request.
 *
 * @param serialized_request The serialized request string.
 * @return A pointer to a `request_t` structure containing the decoded information.
 */
request_t* decode_request(char* serialized_request);

/**
 * Processes a request from a client.
 *
 * This function reads the data sent by a client, decodes it, and handles
 * the request according to its type.
 *
 * @param connfd The connection descriptor between the server and the client.
 */
void process_request(int connfd);

/**
 * Manages requests from the client.
 *
 * This function directs requests to the appropriate functions based on
 * the request type.
 *
 * @param connfd The connection descriptor between the server and the client.
 * @param req Pointer to the `request_t` structure representing the request.
 */
void manage_requests(int connfd, request_t* req);

/**
 * Sends an error message to the client.
 *
 * This function sends an error code and message to the client over the
 * specified connection.
 *
 * @param connfd The connection descriptor between the server and the client.
 * @param error_code The error code to send.
 * @param error_message The error message to send.
 */
void send_error(int connfd, int error_code, char* error_message);

/**
 * Manages errors based on error codes.
 *
 * This function handles error codes and sends appropriate error messages
 * to the client.
 *
 * @param connfd The connection descriptor between the server and the client.
 * @param error_code The error code to handle.
 */
void manage_errors(int connfd, int error_code);

/**
 * Manages sending files to the client.
 *
 * This function sends a file to the client if it exists and is accessible.
 * It reads the file in blocks and sends it over the connection.
 *
 * @param connfd The connection descriptor between the server and the client.
 * @param req Pointer to the `request_t` structure containing the GET request.
 */
void file_manager(int connfd, request_t* req);

/**
 * Displays files available on the server to the client.
 * 
 * This function retrieves the list of files on the server and sends the information to the client.
 * 
 * @param connfd The connection file descriptor for the connection between the server and the client.
 * @param req The request from the client containing necessary information.
 */
void display_files(int connfd, request_t* req);

/**
 * Sends the disconnection status to the master server.
 * 
 * This function notifies the master server about the disconnection of a client.
 * 
 * @param serverfd The connection file descriptor for the connection between the master and the slave.
 * @param server_ip_string The IP address of the server (slave).
 */
void send_disconnection_status(int serverfd, char* server_ip_string);

/**
 * Retrieves the IP address of the server.
 * 
 * This function retrieves the IP address of the server connected to the master.
 * 
 * @param serverfd The connection file descriptor for the connection between the master and the slave.
 * @return A pointer to a string containing the server's IP address.
 */
char* get_server_ip(int serverfd);

#endif