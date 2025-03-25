#include "handlers.h"

void handler_sigint(int sig)
{    
    for (int i = 0; i < NB_PROC; i++) {
        kill(pool[i], SIGINT);
    }

    while(waitpid(-1, NULL, 0) > 0);
    exit(0);
}
