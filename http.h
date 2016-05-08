//
// Created by think on 16-5-7.
//

#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#include<string.h>
#include <stdio.h>
#include <unistd.h>

#define printStr1(s) write(STDOUT_FILENO,(s)->str,(s)->size);
#define printStr2(s) write(STDOUT_FILENO,(s).str,(s).size);

typedef struct Str{
    char *str;
    size_t  size;
} Str_t;

typedef struct Request{
    int fd;
    int method;
    Str_t* uri;
    Str_t* version;
    Str_t* header;
    size_t size;
} Request_t;

Request_t* http_parse(char* content,int fd);

#endif //HTTP_HTTP_H
