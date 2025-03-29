#include "ls.h"

void ls_response(int clientfd)
{
	int message_size;

	/* Read message size */
	if(Rio_readn(clientfd, &message_size, sizeof(message_size)) == 0){
		printf("Lost connection to the server, please try again later.\n");
        exit(0);
	};
	message_size = ntohl(message_size);

	/* Read list of files (ls) from the slave server */
	char* display = malloc(message_size);
	if (!display) {
		perror("display malloc");
		free(display);
		return;
	}

	if(Rio_readn(clientfd, display, message_size) == 0){
		printf("Lost connection to the server, please try again later.\n");
        exit(0);
	}

	display[message_size] = '\0';

	printf("%s", display);
}
