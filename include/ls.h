#ifndef __LS_H__
#define __LS_H__

#include "request.h"
#include <stdio.h>

/**
 * Handles the response for the "ls" command from the slave server.
 * 
 * This function reads the message size and the list of files from the slave server, 
 * and then displays the list of files to the client.
 * 
 * @param clientfd The file descriptor of the connection between the slave server and the client.
 */
void ls_response(int clientfd); 

#endif
