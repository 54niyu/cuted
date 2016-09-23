#include"event.h"
#include<stdlib.h>
#include"kqueue.c"


event_t *new_event(int fd, short type, short flag, void (*cb)(void *), void *data){

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

	Reactor* rc = (Reactor*)malloc(sizeof(Reactor));
	if(rc == NULL)	return NULL;

	rc->op = create_kqueue();

    rc->stop = 0;

    return rc;
}

int reactor_add(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
       return 1;
    }

    return add_kevent(rc->op, ev->fd, ev->flags, ev);
}

int reactor_del(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
        return 1;
    }

    return del_kevent(rc->op,ev->fd,ev->flags,ev);

}

void reactor_loop(Reactor* rc){

    while(!rc->stop){

        int n;
        int i;
        if((n = kevent_dispath(rc->op,NULL))!= -1){

            for(i=0;i<n;i++){
                event_t *ev = (event_t*)(rc->op->events[i].udata);
                ev->cb_function(ev->arg);
		        reactor_del(rc, ev);
            }
        }
    }
}

void reactor_exit(Reactor *rc){

    rc->stop = 1;
}

