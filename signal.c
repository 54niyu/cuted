#include "signal.h"

static int signal_notify_fd;

void handler(int fd) {
    write(signal_notify_fd,(char*)(&fd),sizeof(char));
}

void signal_handler(int fd, void *arg) {
    struct reactor *rc = (struct reactor*)arg;
    int i; 
    char buf[24];
    int ret = read(fd, buf, 24);
    if(ret == -1) {

    }else{
        for(i=0;i<ret;i++){
            int sig = (int)buf[i];
            struct event *sig_ev = rc->signal_map[sig];
            sig_ev->cb_function(sig, sig_ev->arg);
        }
    }
}

int signal_init(struct reactor *rc) {

    // make pip for signal
    if(socketpair(AF_UNIX, SOCK_STREAM, 0 , rc->signal_fd) == -1){
        perror("epoll socketpair\n");
        return -1;
    }
    signal_notify_fd = rc->signal_fd[1]; 

    struct event *ev = new_event(rc->signal_fd[0], 1, CUTE_READ|CUTE_PERSIST, signal_handler, rc);
    reactor_add(rc,ev, NULL);

    return 0;
}
