//
// Created by Bing on 16/9/14.
//

#include <stdlib.h>
#include <sys/event.h>
#include <stdio.h>
#include "event.h"
#include "server.h"

#define NEVENT 64


void* create_kqueue();
int add_kevent(void *op,int fd,short op2,void *data);
int del_kevent(void *op,int fd,short op2,void *data);
int kevent_dispath(void *op,struct timeval *tm);


struct back_ops kq_ops={
        "kqueue",
        create_kqueue,
        add_kevent,
        del_kevent,
        NULL,
        kevent_dispath,
        NULL,
};

struct op_cmd {
    int kq;
    struct kevent *events;
    int events_size;

    struct kevent *changes;
    int changes_size;
};

void* create_kqueue(){

    struct op_cmd *op = (struct op_cmd*)malloc(sizeof(struct op_cmd));

    if( op == NULL) {
      return NULL;
    }

    int kq = kqueue();
    if(kq == -1){
        printf("kqueue error\n");
        free(op);
        return NULL;
    }

    op->kq = kq;

    struct kevent *events = (struct kevent *)malloc(sizeof(struct kevent)*NEVENT);

    if(events == NULL){
        free(op);
        return NULL;
    }

    op->events = events;
    op->events_size = NEVENT;


    struct kevent *changes = (struct kevent *)malloc(sizeof(struct kevent)*NEVENT);

    if(changes == NULL){
        free(op);
        return NULL;
    }

    op->changes = changes;
    op->changes_size = NEVENT;

    return op;
}

int add_kevent(void *back,int fd,short op2,void *data){

    if(back == NULL)	return -1;
    struct op_cmd* op = (struct op_cmd*)back;

	struct kevent ev;

	if ((op2 & CUTE_READ)){
		EV_SET(&ev,fd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,data);
	}
	if ( op2 & CUTE_WRITE){
		EV_SET(&ev,fd,EVFILT_WRITE,EV_ADD|EV_ENABLE,0,0,data);
	}

	return kevent(op->kq, &ev, 1, NULL, 0, NULL);
}

int del_kevent(void *back,int fd,short op2,void *data) {

    if (back == NULL) return -1;
    struct op_cmd* op = (struct op_cmd*)back;


    struct kevent ev;

	if ((op2 & CUTE_READ)){
		EV_SET(&ev,fd,EVFILT_READ,EV_DISABLE|EV_DELETE, 0,0,data);
	}
	if ( op2 & CUTE_WRITE){
		EV_SET(&ev,fd,EVFILT_WRITE,EV_DISABLE|EV_DELETE, 0,0,data);
	}

    return kevent(op->kq, &ev, 1, NULL, 0, NULL);

}

int kevent_dispath(void *back,struct timeval *tm){

    if(back == NULL)  return -1;
    struct op_cmd* op = (struct op_cmd*)back;

    int n=0;
    if ((n=kevent(op->kq,NULL,0,op->events,op->events_size,NULL)) == -1){
        perror("Kevent dispath");
        exit(-1);
    }else{

        int i=0;
        for(i=0;i<n;i++){
            event_t *ev = (event_t*)(op->events[i].udata);
            ev->cb_function(ev->fd,ev->arg);
            if(!(ev->flags & CUTE_PERSIST)){
               del_kevent(op,ev->fd,ev->flags,ev->arg);
            }
        }

    }
    return n;
}
