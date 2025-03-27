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
	printf("sending client to slave server: %s\n", response);

	int response_size_net = htonl(response_size);
	// printf("%d and size %lu\n", ntohl(response_size_net), sizeof(response_size_net));
	Rio_writen(connfd, &response_size_net, 4);
	Rio_writen(connfd, response, response_size);
	
	Close(connfd);
}

/* send slave his IP addr */  
void master_to_slave(int slave_socket, char* slave_ip)
{
	int message_size = 16; 
	char* message = malloc(message_size);

	snprintf(message, message_size, "%s", slave_ip);

	int message_size_net = htonl(message_size);

	Rio_writen(slave_socket, &message_size_net, sizeof(int));
	Rio_writen(slave_socket, message, message_size);
}

int available_slave(slaves* slaves)
{
	for (int i = 0; i < NB_SLAVES; i++) {
		if (slaves[i].nb_clients < slaves[i].pool_size) return slaves[i].id;
	}
	return -1;
}

int get_slave_index(char** IP, char* slave_ip)
{
	for (int i = 0; i < NB_SLAVES; i++) {
		if (strcmp(slave_ip, IP[i]) == 0) return i;
	}

	return -1;
}

/* for slave server disconnections */
void slave_deconnection_status(int slavefd, slaves* slaves, char** IP)
{
    int n;
    char response[MAXLINE];

    Rio_readn(slavefd, &n, sizeof(int));
    n = ntohl(n);

    Rio_readn(slavefd, response, n);
    response[n] = '\0'; 
    printf("%s\n", response);

    char* token = strtok(response, "|");
    if (!token) {
        fprintf(stderr, "Invalid response format\n");
        Close(slavefd);
        return;
    }

    char status[MAXLINE];
    strncpy(status, token, sizeof(status) - 1);
    status[sizeof(status) - 1] = '\0';

    token = strtok(NULL, " \n");
    if (!token) {
        fprintf(stderr, "Invalid response format (missing IP)\n");
        Close(slavefd);
        return;
    }

    int slave_index = get_slave_index(IP, token);

    if (slave_index >= 0 && strcmp(status, "DISCONNECTED") == 0) slaves[slave_index].nb_clients--;

    Close(slavefd);
}


int main(int argc, char** argv)
{
	/* port the client connects to */
	int master_port = 2121; /* default port */
	int listenfd = Open_listenfd(master_port);

	/* port to receive notifications on if slave server connected succesfully to client */
	int slave_port = 2120;
	int slavefd = Open_listenfd(slave_port);

	 /* list of slave serve's IP addresses */
	char* IP[NB_SLAVES] = {
		"152.77.82.179",
		// "152.77.82.180" 
	};

	/* list of slave server's pool sizes */
	int pool_sizes[NB_SLAVES] = { /* A MENTIONNER dans le compte rendu -> dynamic server info with write -> slave server sends his info to the master upon his startup */
		2,
		// 3
	};

	/* mmap for child processes to share the same memory field with the father */
	slaves* slaves = mmap(NULL, NB_SLAVES * sizeof(slaves), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (slaves == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}

	for (int i = 0; i < NB_SLAVES; i++) {
		slaves[i].id = i + 1;
		slaves[i].port = 2121 + slaves[i].id;
		slaves[i].ip_addr = malloc(sizeof(IP[i]));
		slaves[i].ip_addr = IP[i];
		slaves[i].pool_size = pool_sizes[i];
		slaves[i].nb_clients = 0;
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
	        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
	        printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

	        int slave_id;
	        while ((slave_id = available_slave(slaves)) < 0);

        	char* slave_ip = malloc(sizeof(slaves[slave_id - 1].ip_addr)); 
        	slave_ip = IP[slave_id - 1];
        	int slave_port = 2121 + slave_id;

        	master_to_client_response(connfd, slave_id, slave_ip, slave_port);

        	/* check slave connection status */
        	int slave_socket = Accept(slavefd, (SA* )&clientaddr, &clientlen); /* we don't use clientaddr so we re-use is */
        	master_to_slave(slave_socket, slave_ip);

        	Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN); /* same here, -> NOT TO BLOAT CODE */
        	int slave_index = get_slave_index(IP, client_ip_string);
        	
        	slaves[slave_index].nb_clients++;

        	slave_deconnection_status(slave_socket, slaves, IP); /* decrementation is handled inside the slave_connection_status function */
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