#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include "event.h"

void p(void* world){
    printf("%s\n",(char*)world);
}

void q(void* world){

    char buf[200];
    read(0,buf,200);
    printf("%s\n",buf);

}

int main(){

    Reactor *rc = reactor_create();

    event_t *ev = new_event(1,1,CUTE_WRITE,p,(char*)"fuck you");
    event_t *ev2 = new_event(0,1,CUTE_READ,q,NULL);

    reactor_add(rc,ev);
    reactor_add(rc,ev2);

    reactor_loop(rc);

    return 0;
}
