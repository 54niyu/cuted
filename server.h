//
// Created by think on 16-5-8.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <bits/socket.h>
#include "http.h"

typedef struct connect_t{
    int fd;
    struct sockaddr_in addr;
    char* read_buf;
    char* write_buf;
    Request_t *request;
} Connect_t;



#endif //HTTP_SERVER_H
