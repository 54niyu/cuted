//
// Created by think on 16-5-6.
//


#ifdef _Linux
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif

#include "http.h"
#include "server.h"
#include "event.h"

#define MAX_LISTEN 128
#define MAX_EVENT 128

void handle_configure();//配置处理
int bind_server(char *addr, int port);//初始化参数，bind服务器
void main_loop(int serverfd);//主循环
void signal_handler(int sig);//信号处理函数
void register_signal();//信号注册
void addsiggg(int sig);

void register_event(int epollfd, int fd, unsigned int op, unsigned int e);//事件处理


void read_client(int, void *);

void write_client(int, void *);

void accept_client(int, void *);

void main_loop2(int);

int worker = 0;
int workerpid[2] = {0};
int multiprocess = 0;
int num_of_process = 2;
int run = 1;


int main(int argc, char *argv[]) {

    handle_configure();

    int serverfd = bind_server("127.0.0.1", 8081);

    register_signal();//注册事件处理

//    if(sem_init(&sem_socket,1,1)<0) perror("Sem");

    if (multiprocess == 1) {
        //多线程模式
        int i = 0;
        for (; i < 2; i++) {
            int pid = 0;
            if ((pid = fork()) == 0) {
                //子进程
                worker = 1;
                break;
            } else {
                //主进程
                workerpid[i] = pid;
            }
        }
        if (worker == 0) printf("%d  I get two child %d %d\n", getpid(), workerpid[0], workerpid[1]);
    }

    main_loop2(serverfd);//主循环

    close(serverfd);

    return 0;
}

int bind_server(char *addr, int port) {

    int server_fd = 0;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_pton(AF_INET,addr,&server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (server_fd <= 0) {
        perror("Socket:");
        return 1;
    }

    int ret = 0;
    ret = bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ret < 0) {
        perror("Bind:");
        return 2;
    }

    ret = listen(server_fd, MAX_LISTEN);
    if (ret < 0) {
        perror("Listen:");
        return 3;
    }

    printf("--------Server is listening on %s %d  => pid:%d  listening on fd:%d\n", addr, port, getpid(), server_fd);
    return server_fd;
}

void handle_configure() {


}

#ifdef _Linux
void main_loop(int serverfd){

    int epollfd=epoll_create1(0);

    register_event(epollfd,serverfd,EPOLL_CTL_ADD,EPOLLIN);

    struct epoll_event events[MAX_EVENT];
    int number=0;
    int ret=0;

    while(1){

        if(run==0){
            if(worker==0&&multiprocess==1){
                write(STDOUT_FILENO,"Kill",4);
                kill(workerpid[0],15);
                kill(workerpid[1],15);
            }
            printf("%d will stop\n",getpid());
            break;
        }

        ret=epoll_wait(epollfd,&events,MAX_EVENT,-1);
        if(ret<0){
            perror("Epoll_wait");
            continue;
        }

        int i=0;
        for(;i<ret;i++){
           int sfd=((Connect_t*)events[i].data.ptr)->fd;
            if(sfd==serverfd){

                if(events[i].events&EPOLLIN){
               //     sem_wait(&sem_socket);
                    if(number<MAX_EVENT) {
                        struct sockaddr_in client_addr;
                        socklen_t len = sizeof(client_addr);
                        int clientfd = accept(serverfd, (struct sockaddr *) &client_addr, &len);
                        if (clientfd < 0) {
                            perror("Accept");
                            continue;
                        }
                        printf("process %d accept client %d  \n",getpid(),clientfd);

                        Connect_t *con=connect_create();
                        con->fd=clientfd;con->addr=client_addr;


                        struct epoll_event ev;
                        bzero(&ev,sizeof(ev));
                        ev.data.ptr=con;
                        ev.events=EPOLLIN|EPOLLET;

                        if(epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev)==-1){
                            perror("Epoll add:");
                            continue;
                        }
                        setnonblocking(clientfd);
                        number++;
                    }
                //   sem_post(&sem_socket);
                }

            }else{
                if(events[i].events&EPOLLIN){
                    //处理读事件
                    Connect_t *con=(Connect_t*)events[i].data.ptr;


                    int len=read(con->fd,con->read_buf,1024);
                    if(len<=0){
                        printf("Nothing to read why remind me ???\n");
                        sleep(100);
                        break;
                        continue;
                    }else{
                          printf("Receive: %s\n",con->read_buf);
                          printf("Receive %d bytes\n",len);
                    }
                    con->read_buf[len]='\0';
                    con->request->fd=sfd;

                    http_parse(con->read_buf,con->request);

                    struct epoll_event ev;
                    bzero(&ev,sizeof(ev));
                    ev.data.ptr=events[i].data.ptr;
                    ev.events=EPOLLOUT;
                    if(epoll_ctl(epollfd,EPOLL_CTL_MOD,con->fd,&ev)==-1){

                        perror("Epoll mod");
                    }

                }
                else if(events[i].events&EPOLLOUT){

                    //处理写事件
                    response(((Connect_t *)events[i].data.ptr)->request);

                    //删除监听事件
                    connect_delete(events[i].data.ptr);

                    register_event(epollfd,sfd,EPOLL_CTL_DEL,EPOLLIN);

                    close(sfd);//关闭连接

                    number--;
                }else{
                    printf("Something else\n");
                }
            }
        }
    }
}
#else

void accept_client(int fd, void *arg) {

    ssize_t socklen;
    struct sockaddr client_addr;

    int client = accept(fd, &client_addr, &socklen);
    if (client < 0) {
        perror("Accept");
        return;
    }
    printf("process %d accept client %d  \n", getpid(), client);
    setnonblocking(client);
    struct kevent chg;
    Connect_t *con = connect_create();
    con->fd = client;


    event_t *ev = new_event(client, 1, CUTE_READ, read_client, con);
    Reactor *rc = (Reactor *) arg;
    con->backend = rc;
    reactor_add(rc, ev);
}

void read_client(int fd, void *arg) {

    printf("reading from %d..\n", fd);
    Connect_t *con = arg;

    int len = read(con->fd, con->read_buf, 1024);
    if (len <= 0) {
        printf("Nothing to read why remind me ???   %d\n", fd);
        return;
    } else {
        printf("Receive %d bytes\n ", len);
        printf("%s\n", con->read_buf);
    }
    con->read_buf[len] = '\0';
    con->request->fd = fd;

    http_parse(con);

    event_t *ev = new_event(fd, 1, CUTE_WRITE, write_client, con);
    reactor_add((Reactor *) con->backend, ev);
}

void write_client(int fd, void *arg) {

    //处理写事件
    response(arg);
    //删除监听事件
    connect_delete(arg);

    close(fd);//关闭连接
}

void main_loop2(int server_fd) {

    Reactor *rc = reactor_create();
    event_t *sev = new_event(server_fd, 1, CUTE_READ | CUTE_PERSIST, accept_client, rc);

    reactor_add(rc, sev);

    reactor_loop(rc);

}

// void main_loop(int serverfd){

//     int kq =  kqueue();

//     if (kq == -1){
//         perror("Kqueue()");exit(-1);
//     }

//     struct kevent events[MAX_EVENT];

//     size_t nbytes;

//     int error,nev;

//     //setnonblocking(serverfd);

//     struct kevent change;

//     EV_SET(&change,serverfd, EVFILT_READ,EV_ADD|EV_ENABLE,0,0,0);

//     if (kevent(kq,&change,1,NULL,0,NULL)== -1){
//         perror("EV_ADD");exit(-1);
//     }

//     for (;;){

//         if(run==0){
//             if(worker==0&&multiprocess==1){
//                 write(STDOUT_FILENO,"Kill",4);
//                 kill(workerpid[0],15);
//                 kill(workerpid[1],15);
//             }
//             printf("%d will stop\n",getpid());
//             break;
//         }

//         if ((nev= kevent(kq,NULL,0,events,MAX_EVENT,NULL))==-1){
//             perror("Kevent");
//             continue;
//         }

//         for (int i =0;i < nev; i++){
//             int fd = events[i].ident;
//             if (fd == serverfd){

//                 ssize_t  socklen;
//                 struct sockaddr client_addr;

//                 int client = accept(serverfd,&client_addr,&socklen);
//                 if (client < 0) {
//                     perror("Accept");
//                     continue;
//                 }
//                 printf("process %d accept client %d  \n",getpid(),client);
//                 setnonblocking(client);
//                 struct kevent chg;
//                 Connect_t *con=connect_create();
//                 con->fd=client;

//                 EV_SET(&chg,client,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,con);
//                 kevent(kq,&chg,1,NULL,0,NULL);

//             }else{
//                 if (events[i].filter == EVFILT_READ){

//                     printf("reading from %d..\n",fd);
//                     Connect_t *con = events[i].udata;

//                     int len=read(con->fd,con->read_buf,1024);
//                     if(len<=0){
//                         printf("Nothing to read why remind me ???   %d\n",fd);
//                         struct kevent ecv;
//                         EV_SET(&ecv,fd,EVFILT_READ,EV_DISABLE,0,0,con);
//                         kevent(kq,&ecv,1,NULL,0,NULL);
//                     //    sleep(10);
//                         continue;
//                     }else{
//                         printf("Receive %d bytes\n ",len);
//                         printf("%s\n",con->read_buf);
//                     }
//                     con->read_buf[len]='\0';
//                     con->request->fd=fd;

//                     http_parse(con);

//                     struct kevent chg;

//                     EV_SET(&chg,fd,EVFILT_WRITE, EV_ADD,0,0,con);

//                     kevent(kq,&chg,1,NULL,0,NULL);

//                 }else if(events[i].filter == EVFILT_WRITE){

//                     //处理写事件
//                     response(events[i].udata);
//                     //删除监听事件
//                     connect_delete(events[i].udata);

//                     close(fd);//关闭连接

//                 }else{

//                     printf("Something else");

//                 }
//             }
//         }
//     }
//}

#endif

void signal_handler(int sig) {

    printf("Caught %s\n", sys_siglist[sig]);

    switch (sig) {
        case SIGCHLD: {
            int s;
            int pid = wait(&s);
            printf("Child %d exit\n", pid);

        };
            break;
        case SIGTERM:
        case SIGINT:
        case SIGSEGV: {
            run = 0;
        };
    }
    exit(0);
}

void register_signal() {

    addsiggg(SIGCHLD);
    addsiggg(SIGABRT);
    addsiggg(SIGTERM);
    addsiggg(SIGINT);
    addsiggg(SIGSEGV);
}

void addsiggg(int sig) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = signal_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

#ifdef _Linux
void register_event(int epollfd,int fd, unsigned int op,unsigned int e){
    if(op&EPOLL_CTL_DEL){
        epoll_ctl(epollfd,op,fd,NULL);
        return;
    }

    struct epoll_event event;
    bzero(&event,sizeof(event));
    Connect_t* con=connect_create();
    con->fd=fd;
    event.data.ptr=con;
    event.events=e;
    epoll_ctl(epollfd,op,fd,&event);

    return;
}
#else

void register_event(int kfd, int fd, unsigned op, unsigned e) {

}

#endif

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

