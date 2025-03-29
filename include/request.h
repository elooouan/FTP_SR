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

/**
 * Get the type of request based on the command string.
 *
 * @param command The command string (e.g., "get").
 * @return The type of request (0 for "get", -1 for invalid command).
 */
int request_type(char* command);

/**
 * Fills a request structure based on the input command string.
 *
 * @param input The input containing the request data.
 * @param req A pointer to the request structure to be filled.
 * @return A pointer to the filled request structure, or NULL on failure.
 */
request_t* fill_request_struct(char* input, request_t* req);

/**
 * Serializes a request structure into a string for transmission.
 *
 * @param req A pointer to the request structure to be serialized.
 * @return A pointer to the serialized string message.
 */
char* serialize_request(request_t* req);

/**
 * Sends a serialized request to the server.
 *
 * @param clientfd The file descriptor of the connection between the server and the client.
 * @param request A pointer to the serialized request string.
 */
void send_request(int clientfd, char* request);

#endif