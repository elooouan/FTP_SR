#ifndef __MASTER_H__
#define __MASTER_H__

#include "csapp.h"
#include "handlers.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>	

#define NB_SLAVES 2 /* number of slave servers */
#define MAX_NAME_LEN 256

typedef struct slaves
{
	int id; /* slave id */
	int port; /* port = default port (=2121) + id */
	char* ip_addr; /* 255.255.255.255 format */
	int pool_size;
	int nb_clients;
} slaves;

void master_to_client_response(int connfd, int slave_id, char* slave_ip, int slave_port);
int available_slave(slaves* slaves);
int get_slave_index(char** IP, char* slave_ip);
void master_to_slave(int slave_socket, char* slave_ip);

#endif