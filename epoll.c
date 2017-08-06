
// Created by Bing on 17/1/18.
//

#include <sys/epoll.h>
#include "event.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_EVENTS 16


int ep_init(struct reactor* rc);
int ep_add(struct reactor *rc,int fd, short op, void *data);
int ep_del(struct reactor *rc,int fd, short op, void *data);
int ep_dispatch(struct reactor *rc, struct timeval *tm);
int ep_free(struct reactor *rc);

struct back_op ep_op_func= {
        "epoll",
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

int sig_fd = 0;

void signal_handler_2(int sig){
    char buf[20];

    int len = sprintf(buf,"%d",sig);
    buf[len] = '\0';

    write(sig_fd,buf,len);

    return ;
}

int ep_add_sig(struct reactor *rc, int sig, void *data){

    event_t* ev = data;
    rc->signal_map[sig] = ev;

    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = signal_handler_2;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);

    return 0;
}

int ep_del_sig(struct reactor *rc, int sig){

}

void handler(int fd, void *data) {
    write(1, "I am here",10);
    char buf[10];
    int ret = read(fd,buf,10);
    if(ret ==-1){
        perror("Read");
    }else{
    }
    return ;
}

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

	// make pip for signal
    if(socketpair(AF_UNIX, SOCK_STREAM, 0 , rc->signal_fd) == -1){
        perror("epoll socketpair\n");
        free(op->events);
        free(op);
    }
    sig_fd = rc->signal_fd[1];

    struct epoll_event ev;
    struct event *ev2 = (struct event *)malloc(sizeof(struct event));
    ev2->cb_function = handler;
    ev2->fd = rc->signal_fd[0];
    ev2->arg = ev2;

    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr  = ev2;
    int fd = rc->signal_fd[0];
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);

    epoll_ctl(epollfd,EPOLL_CTL_ADD, rc->signal_fd[0], &ev);

    op->ep = epollfd;
	op->events_size = MAX_EVENTS;
	rc->data_back = op;

	return 0;
}

int ep_add(struct reactor *rc,int fd, short op, void *data){
	if( rc == NULL) return -1;
	struct ep_op_data *op_data= (struct ep_op_data*)rc->data_back;
    struct event *_ev= (struct event*)data;
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
    if ( op & CUTE_SIGNAL) {
        ep_add_sig(rc, fd, data);
        return 1;
    }
    if ( op & CUTE_TIMEOUT) {
        write(1,"I am here",10);
        flag |= CUTE_PERSIST;
        struct timer_t* t = (struct timer_t*)malloc(sizeof(struct timer_t));
        if (rc->timer_list == NULL){
            write(1,"add timer",10);
            rc->timer_list = t;
            t->next = NULL;
            t->ev = _ev;
            printf("%ld %ld\n",t->ev->timeout->tv_sec,t->ev->timeout->tv_usec);
        }else{
            struct timer_t* ptr = rc->timer_list;
            while(ptr!=NULL){
                struct timeval *tv = _ev->timeout;
                if( tv->tv_sec > ptr->ev->timeout->tv_sec ||( tv->tv_sec == ptr->ev->timeout->tv_sec && \
                            tv->tv_usec > ptr->ev ->timeout->tv_usec )){
                    ptr = ptr->next;
                }
            }
            if(ptr == NULL){

            }
        }
        return 1;
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
int sub(struct timeval *r,struct timeval *l){
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
    printf("%ld %ld %ld %ld\n",r->tv_sec,r->tv_usec,l->tv_sec,l->tv_usec);

    return 0;
}

int ep_dispatch(struct reactor *rc, struct timeval *tm){
  if(rc == NULL)  return -1;
    struct ep_op_data *op_data = (struct ep_op_data*)rc->data_back;
    int timeout = tm->tv_sec * 1000 + tm->tv_usec/1000;

    int n=0;
    if ((n=epoll_wait(op_data->ep,op_data->events, op_data->events_size, timeout)) == -1 ){
        perror("epoll event dispath");
    }else{
        rc->active_events = (event_t**)malloc(sizeof(event_t*)*n);
        rc->nactive_events = 0;
        int i=0;
        for(i=0;i<n;i++){
            event_t *ev = (event_t*)(op_data->events[i].data.ptr);
            if (op_data->events[i].events & EPOLLERR){
                printf("EOG %d %s",ev->fd, op_data->events[i].data.ptr);
            }else{
//                ev->cb_function(ev->fd,ev->arg);
                rc->active_events[i] = ev;
                rc->nactive_events ++;
            }
        }
        struct timer_t *ptr = rc->timer_list;
        while(ptr!=NULL){
            sub(ptr->ev->timeout, tm);
            if (ptr->ev->timeout->tv_sec <=0 && ptr->ev->timeout->tv_usec<=0){
                ptr->ev->cb_function(ptr->ev->fd,ptr->ev->arg);
                if(ptr->ev->flags & CUTE_PERSIST){
                    
                }else{
                    rc->timer_list = NULL;
                    break;
                }
            }
            ptr = ptr->next;
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

