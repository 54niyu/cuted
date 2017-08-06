//
// Created by Bing on 17/1/16.
//

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


int main(int argc, char **argv) {
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    struct kevent event;
    struct kevent tevent[5];
    struct kevent sevent;
    int kq, fd, ret;
    printf("i am %d\n", getpid());

    if (argc != 2)
        err(EXIT_FAILURE, "Usage: %s path \n", argv[0]);
    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        err(EXIT_FAILURE, "Failed to open '%s'\n", argv[1]);
    printf("I am here\n");

    kq = kqueue();
    if (kq == -1)
        err(EXIT_FAILURE, "kqueue() failed");

    EV_SET(&event, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_DELETE, 0, NULL);
    EV_SET(&sevent, SIGUSR1, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);

    ret = kevent(kq, &event, 1, NULL, 0, NULL);
    if (ret == -1)
        err(EXIT_FAILURE, "kevent register");

    if (event.flags & EV_ERROR)
        err(EXIT_FAILURE, "EVent error: %s", strerror(event.data));

    ret = kevent(kq, &sevent, 1, NULL, 0, NULL);
    if (ret == -1)
        err(EXIT_FAILURE, "kevent register");

    if (sevent.flags & EV_ERROR)
        err(EXIT_FAILURE, "EVent error: %s", strerror(sevent.data));


    for (;;) {
        printf("Listen....\n");
        ret = kevent(kq, NULL, 0, tevent, 1, NULL);
        int i;
        for (i = 0; i < ret; i++) {
            printf("wait over\n");
            if (ret == -1) {
                err(EXIT_FAILURE, "kevent wait");
            } else if (ret > 0) {
                if (tevent[i].filter == EVFILT_SIGNAL) {
                    printf("Signal happend %d  %d\n", SIGUSR1, tevent[i].data);
                } else if (tevent[i].filter == EVFILT_VNODE) {
                    if (tevent[i].fflags & NOTE_DELETE) {
                        printf("Someting was deleted in '%s' %d \n", argv[1], tevent[i].fflags);
                    } else if (tevent[i].fflags & NOTE_WRITE) {
                        printf("SOmeting was written in '%s' %d \n", argv[1], tevent[i].fflags);
                    }
                }
            }
        }
    }
}
