#include "handlers.h"
#include "server.h"

void handler_sigint(int sig)
{    
    for (int i = 0; i < NB_PROC; i++) {
        Kill(pool[i], SIGINT);
    }

    write(STDOUT_FILENO, "\n", 1);
}

// void sigpipe_handler(int sig) {
//     close
// }