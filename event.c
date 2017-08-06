#include"event.h"
#include<stdlib.h>

#define _Linux

#ifdef _Linux
extern struct back_op ep_op_func;
struct back_op *op_func = &ep_op_func;
#else
extern struct back_op kq_op_func;
struct back_op *op_func = &kq_op_func;
#endif

event_t *new_event(int fd, short type, short flag, void (*cb)(int,void *), void *data){

	event_t *ev = (event_t*)malloc(sizeof(event_t));
	if(ev == NULL){
		return NULL;
	}

	ev->fd = fd;
	ev->type = type;
	ev->flags = flag;
	ev->cb_function = cb;
	ev->arg = data;

	return ev;
}


Reactor* reactor_create(){

	struct reactor* rc = (struct reactor*)malloc(sizeof(struct reactor));
	if(rc == NULL)	return NULL;

    rc->func_back = op_func;
    rc->func_back->init(rc);
    rc->stop = 0;

    rc->timer_list = NULL;

    rc->timeout = (struct timeval*)malloc(sizeof(struct timeval));
    rc->timeout->tv_usec = 1000;
    rc->timeout->tv_sec = 0;

    return rc;
}

int reactor_add(Reactor* rc, event_t* ev, struct timeval* tm){

    if (rc == NULL || ev == NULL){
       return 1;
    }

    ev->rc = rc;
    ev->timeout = tm;
    return rc->func_back->add(rc, ev->fd, ev->flags, ev);
}

int reactor_del(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
        return 1;
    }

    return rc->func_back->del(rc,ev->fd,ev->flags,ev);
}

void reactor_exit(Reactor *rc){

    rc->stop = 1;
}

void reactor_loop(Reactor* rc){

    while(!rc->stop){
        rc->func_back->dispatch(rc, rc->timeout);
        int i=0;
        for(;i<rc->nactive_events;i++){
            event_t* ev = rc->active_events[i];
            if(ev->fd == rc->signal_fd[0]){
                // handle signal
            }else{
                ev->cb_function(ev->fd, ev->arg);
            }
        }
    }
}
