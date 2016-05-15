//
// Created by think on 16-5-15.
//

#include "event.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>
#include <wait.h>
#include<semaphore.h>


#define EV_TIME 1
#define EV_READ 2
#define EV_WRITE 8
#define EV_SIGNAL 16


void reator_delete(Reator *rt);
Timer *tm_create();
void tm_delete(Timer *);


Reator* reator_create(){

    Reator* reator=(Reator*)calloc(1,sizeof(Reator));
    if(reator==NULL){
        return NULL;
    }

    reator->epoll_fd=epoll_create1(0);
    if(reator->epoll_fd<0){
        reator_delete(reator);
        return NULL;
    }

    reator->events=(struct epoll_event*)malloc(1024*sizeof(struct epoll_event));
    if(reator->events==NULL){
        reator_delete(reator);
        return NULL;
    }

    int ret=socketpair(PF_UNIX,SOCK_STREAM,0,reator->signal_fd);
    if(ret<0){
        reator_delete(reator);
        return NULL;
    }

    reator->timer_list=tm_create();
    if(reator->timer_list==NULL) {
        reator_delete(reator);
        return NULL;
    }

    return reator;
}

void reator_delete(Reator* rt) {

    if (rt != NULL) {
        if (rt->events != NULL)     free(rt->events);
        if  (rt->timer_list!=NULL)    tm_delete(rt->timer_list);
        close(rt->signal_fd[0]);
        close(rt->signal_fd[1]);
        free(rt);
    }
    return;
}

int reator_add(Reator* rt,int fd,short event,void (*cb)(void* arg)){
    if(rt==NULL)   return 0;

    Event_t *ev;
    ev->type=event;

    switch (event){
        case EV_SIGNAL:{};break;
        case EV_TIME:{};break;
        case EV_READ:{};break;
        case EV_WRITE:{};break;
    }
}
int reator_ctl(Reator* rt,int fd,short event,void (*cb)(void* arg)){

}
int reator_del(Reator* rt,int fd,short event,void (*cb)(void* arg)){

}

void reator_loop(Reator* rt){

    while(rt->stop==1) {

        rt->timeout = gettime();

        int ret = epoll_wait(rt->epoll_fd, rt->events, 1024, rt->timeout);

        if (ret < 0) {
            continue;
        } else if (ret == 0) {
            //时间到
        } else {
            //处理io事件
            int i;
            for (i = 0; i < ret; i++) {
                Event_t *ev = (Event_t *) (rt->events[i].data.ptr);
                ev->ev.io.cb_function(ev->ev.io.arg);
            }
        }

        //处理定时器
    }
}





