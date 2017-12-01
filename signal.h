#ifndef CUTE_SIGNAL_H
#define CUTE_SIGNAL_H

#include "event.h"
#include "unistd.h"
#include <sys/socket.h>
#include "signal.h"
#include <signal.h>
#include <stdio.h>

void handler(int fd) ;
void signal_handler(int fd, void *arg) ;
int signal_init(struct reactor *rc) ;

#endif
