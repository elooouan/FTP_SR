#include "handlers_master.h"

void handler_sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0);
    if (pid == -1 && errno != ECHILD) {
    	unix_error("waitpid error");
    }
    return;
}
