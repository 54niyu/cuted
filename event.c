#include"event.h"
#include<stdlib.h>

extern struct back_op kq_op_func;

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

    rc->func_back = &kq_op_func;
    rc->func_back->init(rc);
    rc->stop = 0;

    return rc;
}

int reactor_add(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
       return 1;
    }

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

        rc->func_back->dispatch(rc, NULL);
    }
}

