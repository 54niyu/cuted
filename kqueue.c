//
// Created by Bing on 16/9/14.
//

#include <stdlib.h>
#include <sys/event.h>
#include <stdio.h>
#include "event.h"
#include "contianer/container.h"

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

struct back_op kq_op_func;
struct back_op *op_func = &kq_op_func;

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

    if(rc == NULL)  return -1;
    struct kq_op_data *op_data = (struct kq_op_data*)rc->data_back;

    short flag = 0;
    intptr_t  _data = 0 ;
    short fflag = 0;
    if ((op & CUTE_READ)){
        flag |= EVFILT_READ;
    }
    if ( op & CUTE_WRITE){
        flag |= EVFILT_WRITE;
    }
    if ( op &CUTE_SIGNAL){
        signal(fd, SIG_IGN);
        flag |= EVFILT_SIGNAL;
    }
    if ( op & CUTE_TIMEOUT){
        struct timeval *tm = ((event_t*)data)->timeout;
        _data = tm->tv_sec * 1000 * 1000  + tm->tv_usec;
        fflag |= NOTE_USECONDS;
        flag |= EVFILT_TIMER;
    }

    // one shot
    if (op & CUTE_PERSIST){
        EV_SET(op_data->changes, fd, flag, EV_ADD, fflag, _data, data);
    }else{
        EV_SET(op_data->changes, fd, flag, EV_ADD|EV_ONESHOT, fflag, _data, data);
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
    if ( op & CUTE_TIMEOUT){
        flag |= EVFILT_TIMER;
    }

    EV_SET(op_data->changes,fd,flag,EV_DISABLE|EV_DELETE, 0,0,data);
    return kevent(op_data->kq, op_data->changes, 1, NULL, 0, NULL);

}

int kq_dispatch(struct reactor *rc,struct timeval *tm){

    if(rc == NULL)  return -1;
    struct kq_op_data *op_data = (struct kq_op_data*)rc->data_back;

    struct timespec tms = {
        rc->timeout->tv_sec,
        rc->timeout->tv_usec * 1000,
    };
    int n=0;
    if ((n=kevent(op_data->kq,NULL,0,op_data->events,op_data->events_size,&tms)) == -1){
        perror("Kevent dispath");
        exit(-1);
    }else{
        rc->active_events = (event_t**)malloc(sizeof(event_t*) * n);
        rc->nactive_events  = 0;
        int i=0;
        for(i=0;i<n;i++){
            event_t *ev = (event_t*)(op_data->events[i].udata);
            if (op_data->events[i].flags & EV_EOF){
                printf("EV_ERROR: %s\n", strerror(op_data->events[i].data));
            }else if( op_data->events[i].flags & EV_ERROR){
                printf("EV_ERROR: %s\n", strerror(op_data->events[i].data));
            }else{
                rc->active_events[i] = ev;
                rc->nactive_events ++ ;
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


int kq_sg_init(struct reactor *rc){
    return 0;
}

int kq_sg_add(struct reactor *rc){
    return 0;
}

int kq_sg_del(struct reactor *rc){
    return 0;
}
