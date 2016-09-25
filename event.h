//
// Created by think on 16-5-15.
//

#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H

#include <time.h>
#include <sys/_types/_timeval.h>
//#include "kqueue.c"

#define CUTE_IO 1
#define CUTE_SIG 2
#define CUTE_TM 4

#define CUTE_READ 1
#define CUTE_WRITE 2
#define CUTE_SIGNAL 4
#define CUTE_TIME 8
#define CUTE_PERSIST 16

typedef struct timer_t{

} Timer;

typedef struct event{

    short type;
    short flags;
    int fd;
    void (*cb_function)(int fd,void *arg);
    void *arg;
    struct timeval* timeout;

} event_t;


typedef struct reactor{

    //event
    struct back_ops *func_back;
    void* data_back;

    struct op_cmd * op;

    //signal
    int signal_fd[2];//信号处理

    //timer
    Timer *timer_list;//定时器

    struct timeval* timeout;

    // reactor control
    int stop;//结束标志

} Reactor;

struct back_ops{

    char* name;

    void* (*create)();
    int (*add)(void* backend,int fd,short flag,void* data);
    int (*del)(void* backend,int fd,short flag,void* data);
    int (*mod)(void* backend,int fd,short flag,void* data);
    int (*dispatch)(void* backend,struct timeval *time);
    int (*free)(void* backend);

};


event_t *new_event(int fd,short type,short flag,void (*cb)(int fd,void *),void *data);

Reactor* reactor_create();
int reactor_add(Reactor* rt,event_t* ev);
int reactor_del(Reactor* rt,event_t *ev);
void reactor_loop(Reactor* rt);

extern struct back_ops kq_ops;

#endif //HTTP_EVENT_H
