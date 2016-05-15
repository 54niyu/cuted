//
// Created by think on 16-5-15.
//

#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H

typedef struct event{
    int type;
    //io
    union{
        struct io{
            int fd;
            void (cb_function)(void *arg);
            void *arg;
        } io;
        struct signal{
            int sig;
            void (cb_function)(void *arg);
            void *arg;
        } sg;
        struct timer{
            struct timeval va;
            void (cb_function)(void *arg);
            void *arg;
        } tm;
    } ev;
} Event_t;

typedef struct timer{


} Timer;

typedef struct reator{

    int epoll_fd;

    struct epoll_event *events;

    int signal_fd[2];//信号处理

    Timer *timer_list;//定时器

    int timeout;

    int stop;//结束标志

} Reator;

#endif //HTTP_EVENT_H
