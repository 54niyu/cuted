//
// Created by think on 16-5-15.
//

#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H

#include <time.h>
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
    void (*cb_function)(void *arg);
    void *arg;
    struct timeval* timeout;

} event_t;


typedef struct reactor{

    struct op_cmd * op;

    int signal_fd[2];//信号处理

    Timer *timer_list;//定时器

    struct timeval* timeout;

    int stop;//结束标志

} Reactor;

event_t *new_event(int fd,short type,short flag,void (*cb)(void *),void *data);


Reactor* reactor_create();
int reactor_add(Reactor* rt,event_t* ev);
int reactor_del(Reactor* rt,event_t *ev);
void reactor_loop(Reactor* rt);



#endif //HTTP_EVENT_H
