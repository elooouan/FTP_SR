#ifndef __SERVER_H__
#define __SERVER_H__

#define NB_PROC 3 /* number of processes (children) in process pool */
#define FILENAME_MAXSIZE 20 /* filename's max size */

typedef enum {
    GET
} typereq_t;

typedef struct request_t {
    typereq_t type;
    char name[FILENAME_MAXSIZE];
} request_t;

#endif