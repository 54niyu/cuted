//
// Created by Bing on 17/1/18.
//

#include <sys/epoll.h>
#include "event.h"
#include <stdlib.h>
#include <stdio.h>

#define MAX_EVENTS 16


int ep_init(struct reactor* rc);
int ep_add(struct reactor *rc,int fd, short op, void *data);
int ep_del(struct reactor *rc,int fd, short op, void *data);
int ep_dispatch(struct reactor *rc, struct timeval *tm);
int ep_free(struct reactor *rc);

struct back_op ep_op_func= {
        "kqueue",
        ep_init,
        ep_add,
        ep_del,
        ep_dispatch,
        ep_free,
};

struct ep_op_data {
    int ep;
    struct epoll_event *events;
    int events_size;

};

int ep_init(struct reactor* rc){

	struct ep_op_data *op = (struct ep_op_data*)malloc(sizeof(struct ep_op_data));
	if ( op  == NULL){
		printf("epoll malloc error\n");
		return -1;
	}


	int epollfd = epoll_create1(0);
	if ( epollfd < 0){
		printf("epoll create\n");
		free(op);
		return -1;
	}

	op->events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*MAX_EVENTS);
	if(op->events == NULL){
		printf("epoll malloc events\n");
		free(op);
		return -1;
	}

	op->ep = epollfd;
	op->events_size = MAX_EVENTS;
	rc->data_back = op;

	return 0;
}
int ep_add(struct reactor *rc,int fd, short op, void *data){
	if( rc == NULL) return -1;
	struct ep_op_data *op_data= (struct ep_op_data*)rc->data_back;

	struct epoll_event ev;

	short flag = 0;
	if ( op & CUTE_READ){
		flag |= EPOLLIN;
	}
	if ( op & CUTE_WRITE){
		flag |= EPOLLOUT;
	}

	if (! (op & CUTE_PERSIST)){
		flag |= EPOLLONESHOT;
	}

	ev.data.ptr = data;
	ev.events = flag;

	return epoll_ctl(op_data->ep, EPOLL_CTL_ADD, fd, &ev);
}

int ep_del(struct reactor *rc,int fd, short op, void *data){

	if( rc == NULL) return -1;
	struct ep_op_data *op_data= (struct ep_op_data*)rc->data_back;

	struct epoll_event ev;

	short flag = 0;
	if ( op & CUTE_READ){
		flag |= EPOLLIN;
	}
	if ( op & CUTE_WRITE){
		flag |= EPOLLOUT;
	}

	ev.data.ptr = data;
	ev.events = flag;

	return epoll_ctl(op_data->ep, EPOLL_CTL_DEL, fd, &ev);
}

int ep_dispatch(struct reactor *rc, struct timeval *tm){
  if(rc == NULL)  return -1;
    struct ep_op_data *op_data = (struct ep_op_data*)rc->data_back;

    int n=0;
    if ((n=epoll_wait(op_data->ep,op_data->events, op_data->events_size, -1)) == -1 ){
        perror("epoll event dispath");
        exit(-1);
    }else{
        int i=0;
        for(i=0;i<n;i++){
            event_t *ev = (event_t*)(op_data->events[i].data.ptr);
            if (op_data->events[i].events & EPOLLERR){
                printf("EOG %d %s",ev->fd, op_data->events[i].data.ptr);
            }else{
                ev->cb_function(ev->fd,ev->arg);
            }
        }
    }
    return n;
}

int ep_free(struct reactor *rc){

	struct ep_op_data *op =rc->data_back;

	free(op->events);
	free(op);

	printf("Free epoll backend \n");

	return 0;
}


int sg_init(struct reactor *rc){

}

int sg_add(struct reactor *rc){
}

int sg_del(struct reactor *rc){
}

