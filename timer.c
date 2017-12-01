#include "timer.h"

int timer_active(struct reactor *rc, struct timeval* tm) {
    struct timer_t *ptr = rc->timer_list;
    while(ptr!=NULL){
        int zero = timer_ev_sub(ptr->ev->timeout, tm);
        if(zero == 1) {
            ptr->ev->cb_function(ptr->ev->fd,ptr->ev->arg);
            if(ptr->ev->flags & CUTE_PERSIST){
                // need to reset timeout
                ptr->ev->timeout->tv_sec =5;
            }else{
                timer_del(rc, ptr->ev);
            }
        }
        ptr = ptr->next;
    }
    return 0;
}

int timer_add(struct reactor *rc, struct event *ev){

    if (rc->timer_list == NULL){
        struct timer_t* t = (struct timer_t*)malloc(sizeof(struct timer_t));
        rc->timer_list = t;
        t->next = NULL;
        t->ev = ev;
    }

    struct timer_t* ptr = rc->timer_list;
    while(ptr->next !=NULL){
        struct timeval *tv = ev->timeout;
        if( tv->tv_sec > ptr->next->ev->timeout->tv_sec || \
                ( tv->tv_sec == ptr->next->ev->timeout->tv_sec && \
                  tv->tv_usec > ptr->next->ev ->timeout->tv_usec )){
            ptr = ptr->next;
        }
    }
    struct timer_t* t = (struct timer_t*)malloc(sizeof(struct timer_t));
    t->next = ptr->next;
    t->ev = ev;
    return 0;
}

int timer_del(struct reactor *rc, struct event *ev){

    struct timer_t* ptr = rc->timer_list;
    while(ptr->next != NULL) {
        if(ptr->next->ev == ev) {
            struct timer_t* p = ptr->next;
            ptr->next = p->next;
            free(p->ev);
            free(p);
        } 
    }
    return 0;
}

int timer_ev_sub(struct timeval *r,struct timeval *l){
    if(l->tv_usec <= r->tv_usec){
        r->tv_usec -= l->tv_usec;
    }else{
        if(r->tv_sec > 0){
            r->tv_sec -= 1;
            r->tv_usec += 1000 * 1000;
            r->tv_usec -= l->tv_usec;
        }else{
            r->tv_usec -= l->tv_usec;
        }
    }
    r->tv_sec -= l->tv_sec;

    if(r->tv_sec <= 0 && r->tv_usec <= 0){
        return 1;
    }
    return 0;
}


