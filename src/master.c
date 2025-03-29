/*
 * master.c - An ftp master server
 */
#include "master.h"

void master_to_client_response(int connfd, int slave_id, char* slave_ip, int slave_port)
{
	int response_size = 21; 
	char* response = malloc(response_size);

	snprintf(response, response_size, "%s|%d", slave_ip, slave_port);
	printf("sending client to slave server: %s\n", response);

	int response_size_net = htonl(response_size);

	Rio_writen(connfd, &response_size_net, 4);
	Rio_writen(connfd, response, response_size);
	
	Close(connfd);
}

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

void slave_disconnection_status(int slavefd, slaves* slaves, char** IP)
{
    int n;
    char response[MAXLINE];

    Rio_readn(slavefd, &n, sizeof(int)); // Read the size of the incoming response from the slave
    n = ntohl(n);

    Rio_readn(slavefd, response, n); // Read the actual response data from the slave server
    response[n] = '\0'; 
    printf("%s\n", response);

    char* token = strtok(response, "|"); // Tokenize the response using the delimiter
    if (!token) {
        fprintf(stderr, "Invalid response format\n");
        Close(slavefd);
        return;
    }

    char status[MAXLINE];
    strncpy(status, token, sizeof(status) - 1); // Extract the status of the slave ("DISCONNECTED")
    status[sizeof(status) - 1] = '\0';

    token = strtok(NULL, " \n"); // Get the IP address of the slave
    if (!token) {
        fprintf(stderr, "Invalid response format (missing IP)\n");
        Close(slavefd);
        return;
    }

	/* Get the index of the slave in the IP address list */
    int slave_index = get_slave_index(IP, token); 

	/* If the slave is found and its status is "DISCONNECTED", decrement the number of clients connected to it */
    if (slave_index >= 0 && strcmp(status, "DISCONNECTED") == 0) slaves[slave_index].nb_clients--;

    Close(slavefd); // Close the connection to the slave server
}


int main(int argc, char** argv)
{
	Signal(SIGCHLD, handler_sigchld);

	/* Port the client connects to */
	int master_port = 2168; // Default port
	int listenfd = Open_listenfd(master_port);

	/* Port to receive notifications from the slave server*/
	int slave_port = 2120;
	int slavefd = Open_listenfd(slave_port);

	/* List of slave serve's IP addresses */
	char* IP[NB_SLAVES] = {
		"152.77.81.20",
		"152.77.82.180" 
	};

	/* List of slave server's pool sizes */
	int pool_sizes[NB_SLAVES] = {
		2,
		3
	};

	/* Using mmap to create a shared memory region for child processes to access the same memory as the parent */
	slaves* slaves = mmap(NULL, NB_SLAVES * sizeof(slaves), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (slaves == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}

	/* Initialize the slave server structure */
	for (int i = 0; i < NB_SLAVES; i++) {
		slaves[i].id = i + 1;
		slaves[i].port = master_port + slaves[i].id;
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
        /* Waiting for a client to connect */
        connfd = Accept(listenfd, (SA* )&clientaddr, &clientlen);
        
		/* Create a new child process to handle the client's connection */
        pid_t pid = Fork();

        /* In the child process, handle the client's request and redirect them to a slave server */
        if (pid == 0) {
	        /* client connected */
	        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
	        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
	        printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

	        int slave_id;
			/* Find an available slave server to handle the client */
	        while ((slave_id = available_slave(slaves)) < 0);

			/* Assign the selected slave's IP address and port */
        	char* slave_ip = malloc(sizeof(slaves[slave_id - 1].ip_addr)); 
        	slave_ip = IP[slave_id - 1];
        	int slave_port = master_port + slave_id;

			/* Send the selected slave's information (IP and port) to the client */
        	master_to_client_response(connfd, slave_id, slave_ip, slave_port);

        	/* check slave connection status */
        	int slave_socket = Accept(slavefd, (SA* )&clientaddr, &clientlen); /* we don't use clientaddr so we re-use is */
        	master_to_slave(slave_socket, slave_ip);

			/* Update the slave's client count based on the client's IP address */
        	Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN); /* same here, -> NOT TO BLOAT CODE */
        	int slave_index = get_slave_index(IP, client_ip_string);
        	
			/* Increment the number of clients connected to the slave */
        	slaves[slave_index].nb_clients++;

			/* Handle the slave's disconnection status */
        	slave_disconnection_status(slave_socket, slaves, IP); // Decrement the client count if the client disconnects from the server
			exit(0);	
        } else {
			/* Close the connection in the parent process after handing it off to the child */
        	Close(connfd);
        }
    }
}