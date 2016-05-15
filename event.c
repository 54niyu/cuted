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






