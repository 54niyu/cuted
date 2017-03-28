//
// Created by Bing on 16/9/14.
//

#include <stdlib.h>
#include <sys/event.h>
#include <stdio.h>
#include "event.h"

#define NEVENT 64

int kq_init(struct reactor* rc);
int kq_add(struct reactor *rc,int fd, short op, void *data);
int kq_del(struct reactor *rc,int fd, short op, void *data);
int kq_dispatch(struct reactor *rc, struct timeval *tm);
int kq_free(struct reactor *rc);

struct back_op kq_op_func= {
        "kqueue",
        kq_init,
        kq_add,
        kq_del,
        kq_dispatch,
        kq_free,
};

struct kq_op_data {
    int kq;
    struct kevent *events;
    int events_size;

    struct kevent *changes;
    int changes_size;
};


int kq_init(struct reactor *rc){

    struct kq_op_data *op = (struct kq_op_data*)malloc(sizeof(struct kq_op_data));

    if( op == NULL) {
        printf("kqueue malloc error\n");
        return -1;
    }

    int kq = kqueue();
    if(kq == -1){
        printf("kqueue error\n");
        free(op);
        return -1;
    }

    op->kq = kq;
    rc->data_back = op;

    struct kevent *events = (struct kevent *)malloc(sizeof(struct kevent)*NEVENT);

    if(events == NULL){
        free(op);
        return -1;
    }

    op->events = events;
    op->events_size = NEVENT;

    struct kevent *changes = (struct kevent *)malloc(sizeof(struct kevent)*NEVENT);

    if(changes == NULL){
        free(op->events);
        free(op);
        return -1;
    }

    op->changes = changes;
    op->changes_size = NEVENT;

    return 0;
}

int kq_add(struct reactor *rc,int fd,short op,void *data){

    if(rc == NULL)	return -1;
    struct kq_op_data *op_data = (struct kq_op_data*)rc->data_back;

    short flag = 0;
	if ((op & CUTE_READ)){
        flag |= EVFILT_READ;
	}
	if ( op & CUTE_WRITE){
        flag |= EVFILT_WRITE;
	}
    if ( op &CUTE_SIGNAL){
        flag |= EVFILT_SIGNAL;
    }

    // one shot
    if (op & CUTE_PERSIST){
        EV_SET(op_data->changes, fd, flag, EV_ADD, 0, 0, data);
    }else{
        EV_SET(op_data->changes, fd, flag, EV_ADD|EV_ONESHOT, 0, 0, data);
    }

	return kevent(op_data->kq, op_data->changes, 1, NULL, 0, NULL);
}

int kq_del(struct reactor *rc,int fd,short op,void *data) {

    if (rc == NULL) return -1;
    struct kq_op_data* op_data = (struct kq_op_data*)rc->data_back;

    short flag = 0;

	if ((op & CUTE_READ)){
        flag |= EVFILT_READ;
	}
	if (op & CUTE_WRITE){
        flag |= EVFILT_WRITE;
	}
    if (op & CUTE_SIGNAL){
        flag |= EVFILT_SIGNAL;
    }

    EV_SET(op_data->changes,fd,flag,EV_DISABLE|EV_DELETE, 0,0,data);
    return kevent(op_data->kq, op_data->changes, 1, NULL, 0, NULL);

}

int kq_dispatch(struct reactor *rc,struct timeval *tm){

    if(rc == NULL)  return -1;
    struct kq_op_data *op_data = (struct kq_op_data*)rc->data_back;

    int n=0;
    if ((n=kevent(op_data->kq,NULL,0,op_data->events,op_data->events_size,NULL)) == -1){
        perror("Kevent dispath");
        exit(-1);
    }else{
        int i=0;
        for(i=0;i<n;i++){
            event_t *ev = (event_t*)(op_data->events[i].udata);
            if (op_data->events[i].flags & EV_EOF){
                printf("EOG %d %s",ev->fd, op_data->events[i].fflags);
            }else{
                ev->cb_function(ev->fd,ev->arg);
            }
        }
    }
    return n;
}

int kq_free(struct reactor *rc){
    struct kq_op_data *op_data = rc->data_back;

    free(op_data->events);
    free(op_data->changes);
    free(op_data);

    printf("Free kqueue backend \n");
    return 0;
}


int sg_init(struct reactor *rc){

}

int sg_add(struct reactor *rc){

}

int sg_del(struct reactor *rc){

}