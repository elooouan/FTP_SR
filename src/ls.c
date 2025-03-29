#include "ls.h"

void ls_response(int clientfd)
{
	int message_size;

	/* Read message size */
	Rio_readn(clientfd, &message_size, sizeof(message_size));
	message_size = ntohl(message_size);

	/* Read list of files (ls) from the slave server */
	char* display = malloc(message_size);
	if (!display) {
		perror("display malloc");
		free(display);
		return;
	}

	Rio_readn(clientfd, display, message_size);

	display[message_size] = '\0';

	printf("%s", display);
}
