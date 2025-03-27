#ifndef __SERVER_H__
#define __SERVER_H__

#include "handlers.h"
#include "get.h"
#include "ls.h"

void response_treatment(int clientfd, request_t* request);
void read_from_master(int clientfd);

#endif