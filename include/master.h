#ifndef __MASTER_H__
#define __MASTER_H__

#include "csapp.h"
#include "handlers_master.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>	

#define NB_SLAVES 2 /* number of slave servers */
#define MAX_NAME_LEN 256

/**
 * Structure representing a slave server.
 */
typedef struct slaves 
{
    int id;             /**< Unique identifier for the slave server */
    int port;           /**< Port number on which the slave server is listening */
    char* ip_addr;      /**< IP address of the slave server */
    int pool_size;      /**< Maximum number of clients the slave can handle */
    int nb_clients;     /**< Number of clients currently connected to the slave */
} slaves;

/**
 * Sends the slave server information to the client.
 * 
 * This function constructs and sends a response to the client, 
 * providing information about the selected slave server (IP and Port).
 * 
 * @param connfd The connection file descriptor for the connection between the master and the client.
 * @param slave_id The ID of the selected slave server.
 * @param slave_ip The IP address of the selected slave server.
 * @param slave_port The port of the selected slave server.
 */
void master_to_client_response(int connfd, int slave_id, char* slave_ip, int slave_port);

/**
 * Sends the slave server's IP address to the slave.
 * 
 * This function sends the IP address of the slave server to the slave server itself.
 * 
 * @param slave_socket The file descriptor for the connection between the master and slave.
 * @param slave_ip The IP address of the slave server itself.
 */
void master_to_slave(int slave_socket, char* slave_ip);

/**
 * Finds an available slave server.
 * 
 * This function searches for a slave server with available capacity to handle more clients.
 * 
 * @param slaves The array of slave servers.
 * @return The ID of an available slave server, or -1 if none is available.
 */
int available_slave(slaves* slaves);

/**
 * Retrieves the index of a slave server based on its IP address.
 * 
 * This function searches for a slave server in the IP list and returns its index.
 * 
 * @param IP The list of IP addresses of the slave servers.
 * @param slave_ip The IP address of the slave server to search for.
 * @return The index of the slave server in the list, or -1 if not found.
 */
int get_slave_index(char** IP, char* slave_ip);

/**
 * Handles the disconnection status of a slave server.
 * 
 * This function processes the disconnection notification from a slave server and updates the slave's client count accordingly.
 * 
 * @param slavefd The file descriptor for the connection between the master and the slave.
 * @param slaves The array of slave servers.
 * @param IP The list of IP addresses of the slave servers.
 */
void slave_disconnection_status(int slavefd, slaves* slaves, char** IP);

#endif