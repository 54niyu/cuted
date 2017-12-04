//
// Created by think on 16-5-7.
//

#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#define printStr1(s) write(STDOUT_FILENO,(s)->str,(s)->size);
#define printStr2(s) write(STDOUT_FILENO,(s).str,(s).size);


typedef struct Str{
    char *str;
    size_t  size;
} Str_t;


typedef struct Request{
    int fd;
    int method;
    Str_t uri;
    Str_t version;
    Str_t* header;
    size_t size;
    Str_t *parametar;
    Str_t psize;
    Str_t body;

    int stat_code;
    Str_t buf;
    int file_fd;
    int file_size;
} request_t;

typedef struct connect{
    int fd;
    struct sockaddr_in addr;
    char* read_buf;
    char* write_buf;
    request_t *request;
    void* backend;
} connect_t;

request_t* request_create();
void request_delete(request_t *req);
request_t* http_parse(connect_t* con);
void response(connect_t* con);
connect_t *connect_create();
void connect_delete(connect_t *con);
#endif //HTTP_HTTP_H
