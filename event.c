#include"event.h"
#include<stdlib.h>
#include"kqueue.c"


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

	Reactor* rc = (Reactor*)malloc(sizeof(Reactor));
	if(rc == NULL)	return NULL;

	rc->op = kq_ops.create(); // create_kqueue();
    rc->func_back = &kq_ops;
    rc->data_back = rc->func_back->create();

    rc->stop = 0;

    return rc;
}

int reactor_add(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
       return 1;
    }

    return rc->func_back->add(rc->data_back, ev->fd, ev->flags, ev);
}

int reactor_del(Reactor* rc, event_t* ev){

    if (rc == NULL || ev == NULL){
        return 1;
    }

    return rc->func_back->del(rc->data_back,ev->fd,ev->flags,ev);

}

void reactor_exit(Reactor *rc){

    rc->stop = 1;
}

void reactor_loop(Reactor* rc){

    while(!rc->stop){

        int n;
        int i;
        if((n = (rc->func_back->dispatch)(rc->data_back,NULL))!= -1){
//
//            for(i=0;i<n;i++){
//                event_t *ev = (event_t*)(rc->data_back->events[i].udata);
//                ev->cb_function(ev->arg);
//		        reactor_del(rc, ev);
//            }
        }
       // reactor_exit(rc);
    }
}

