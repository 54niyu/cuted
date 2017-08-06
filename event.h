//
// Created by think on 16-5-15.
//

#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H

#include <time.h>
#include "contianer/container.h"

#define CUTE_READ 1
#define CUTE_WRITE 2
#define CUTE_SIGNAL 4
#define CUTE_TIMEOUT 8
#define CUTE_PERSIST 16

struct timer_t {
    struct timer_t *next;
    struct event *ev;
};

typedef void (*event_cb)(int fd, void *arg);

typedef struct event {

    struct reactor * rc;

    short type;
    short flags;
    int fd;
    void (*cb_function)(int fd, void *arg);
    void *arg;
    struct timeval *timeout;

} event_t;

typedef struct reactor {

    //event
    struct back_op *func_back;
    void *data_back;

    // active events
    event_t **active_events;
    int nactive_events;

    //signal
    int signal_fd[2];//信号处理
    struct event *signal_map[32];

    //timer
    struct timer_t *timer_list;//定时器

    struct timeval *timeout;

    // reactor control
    int stop;//结束标志

} Reactor;

struct back_op {

    char *name;

    int (*init)(struct reactor *base);

    int (*add)(struct reactor *base, int fd, short flag, void *data);

    int (*del)(struct reactor *base, int fd, short flag, void *data);

    int (*dispatch)(struct reactor *base, struct timeval *time);

    int (*free)(struct reactor *base);

};


event_t *new_event(int fd, short type, short flag, void (*cb)(int fd, void *), void *data);

Reactor *reactor_create();

int reactor_add(Reactor *rt, event_t *ev, struct timeval *tm);

int reactor_del(Reactor *rt, event_t *ev);

void reactor_loop(Reactor *rt);

extern struct back_ops kq_ops;

#endif //HTTP_EVENT_H
