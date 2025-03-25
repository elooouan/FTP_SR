/*
 * master.c - An ftp master server
 */
#include "master.h"

void master_to_client_response(int connfd, int slave_id, char* slave_ip, int slave_port)
{
	int response_size = 21; 
	// sizeof(slave_ip) + 1 + sizeof(slave_port) + 1;
	char* response = malloc(response_size);

	snprintf(response, response_size, "%s|%d", slave_ip, slave_port);
	printf("%s\n", response);

	int response_size_net = htonl(response_size);
	// printf("%d and size %lu\n", ntohl(response_size_net), sizeof(response_size_net));
	Rio_writen(connfd, &response_size_net, 4);
	Rio_writen(connfd, response, response_size);
	Close(connfd);
}

int available_slave(slaves* slaves)
{
	for (int i = 0; i < NB_SLAVES; i++) {
		if (slaves[i].is_used == 0) {
			return slaves[i].id;
		}
	}
	return -1;
}

int main(int argc, char** argv)
{
	int port = 2121; /* default port */
	int listenfd = Open_listenfd(port);

	/* list of slave server's IPs */
	char* IP[NB_SLAVES] = {
		"152.77.82.179",
		"152.77.82.180" 
	};

	slaves* slaves = malloc(NB_SLAVES * sizeof(slaves));
	if (!slaves) {
		perror("slaves malloc");
		exit(1);
	}

	for (int i = 0; i < NB_SLAVES; i++) {
		slaves[i].id = i + 1;
		slaves[i].port = 2121 + slaves[i].id;
		slaves[i].ip_addr = malloc(sizeof(IP[i]));
		slaves[i].ip_addr = IP[i];
		slaves[i].is_used = 0;
	}

    int connfd;	
	struct sockaddr_in clientaddr;
	char client_ip_string[INET_ADDRSTRLEN];
	char client_hostname[MAX_NAME_LEN];
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
 
    while (1) {
        /* waiting for connection */
        connfd = Accept(listenfd, (SA* )&clientaddr, &clientlen);
        
        pid_t pid = Fork();

        /* For simultaneous connections -> if connection with client, handle it's redirection in the child then kill it */
        if (pid == 0) {
	        /* client connected */
	        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
	        
	        printf("server connected to %s (%s) | client info: %u\n", client_hostname,
	            client_ip_string, clientaddr.sin_addr.s_addr);

	        int slave_id;
	        while ((slave_id = available_slave(slaves)) < 0);

        	char* slave_ip = malloc(sizeof(slaves[slave_id - 1].ip_addr)); 
        	slave_ip = IP[slave_id - 1];
        	int slave_port = 2121 + slave_id;

        	master_to_client_response(connfd, slave_id, slave_ip, slave_port);
        } else {
        	Close(connfd);
        }

        // /* gets connection */
        // connection_closed = 0;      

        /* traitement */
        // if (!connection_closed) Close(connfd);
        // printf("Connection to (%s) closed\n", client_ip_string);
    }
}