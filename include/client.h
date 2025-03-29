#ifndef __SERVER_H__
#define __SERVER_H__

#include "handlers.h"
#include "get.h"

void response_treatment(int clientfd, request_t* request);

#endif