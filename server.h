//
// Created by think on 16-5-8.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http.h"

typedef struct connect_t{
    int fd;
    Request_t *request;
};



#endif //HTTP_SERVER_H
