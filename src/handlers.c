#include "handlers.h"
#include "server.h"

void handler_sigint(int sig)
{    
    for (int i = 0; i < NB_PROC; i++) {
        kill(pool[i], 9);
    }

    kill(getpgid(getpid()), 9);
}