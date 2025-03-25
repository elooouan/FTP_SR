#include "handlers.h"
#include "server.h"

void handler_sigint(int sig)
{    
    for (int i = 0; i < NB_PROC; i++) {
        kill(pool[i], SIGINT);
        pool[i] = 0;
    }

    while(!empty_pool(pool));
    exit(0);
}

int empty_pool(int pool[]){
	for(int i = 0; i < NB_PROC; i++){
		if(pool[i] != 0) return 0;
	}
	return 1;
}
