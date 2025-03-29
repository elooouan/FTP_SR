#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "get.h"
#include "ls.h"

/**
 * Handles the server response based on the request type.
 * 
 * This function processes the response from the server based on
 * the type of the request (e.g., GET or LS).
 * 
 * @param clientfd The file descriptor for the connection between the server and the client.
 * @param request Pointer to the request structure containing the request details.
 */
void response_treatment(int clientfd, request_t* request);

/**
 * Reads a message from the server (master) to the client.
 * 
 * This function receives a message from the server (master),
 * and stores it in the provided message buffer.
 * 
 * The message contains information on the slave server (IP and port).
 * 
 * @param clientfd The file descriptor for the client connection.
 * @param message Pointer to a pointer to the message buffer to store the received message.
 */
void message_from_master(int clientfd, char** message);

/**
 * Extracts slave information (IP and port) from the server message.
 * 
 * This function parses the received message and extracts the slave's
 * IP address and port number.
 * 
 * @param message The server message containing the slave information.
 * @param slave_ip Buffer to store the extracted slave IP.
 * @param slave_port Pointer to store the extracted slave port.
 */
void extract_slave_info(char* message, char* slave_ip, int* slave_port);

#endif