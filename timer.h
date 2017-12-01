
#ifndef CUTE_TIMER_H
#define CUTE_TIMER_H
#include "event.h"

int timer_ev_sub(struct timeval *r,struct timeval *l);
int timer_del(struct reactor* rc, struct event *ev);
int timer_active(struct reactor *rc, struct timeval* tm);
int timer_add(struct reactor *rc, struct event *ev);
int timer_del(struct reactor *rc, struct event *ev);

#endif
