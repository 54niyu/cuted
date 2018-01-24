#ifndef CONNECT_H
#define CONNECT_H

#include <stdio.h>

struct ct_connect {
    int fd;
    struct sockaddr_in l_addr;
    struct sockaddr_in r_addr;
    char* read_buf;
    char* write_buf;
};

int cn_write(struct ct_connect *cn,char* src, int sz);
int cn_read(struct ct_connect *cn,char* dst, int sz);
int cn_close(struct ct_connect *cn);

#endif
