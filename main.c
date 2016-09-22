#include<stdlib.h>
#include<stdio.h>
#include "event.h"

void p(void* world){

    printf("%s\n",(char*)world);

}

int main(){

    Reactor *rc = reactor_create();

    event_t *ev = new_event(0,1,CUTE_WRITE,p,(char*)"fuck you");

    reactor_add(rc,ev);

    reactor_loop(rc);

    return 0;
}
