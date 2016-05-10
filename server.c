//
// Created by think on 16-5-6.
//
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>
#include <wait.h>
#include<semaphore.h>

#include "http.h"
#include "server.h"

#define MAX_LISTEN 128
#define MAX_EVENT 128

void hander_configure();//配置处理
int bind_server(char *addr, int port);//初始化参数，bind服务器
void main_loop(int serverfd);//主循环
void signal_hander(int sig);//信号处理函数
void register_signal();//信号注册
void register_event(int epollfd,int fd, unsigned int op, unsigned int e);//事件处理
Connect_t *connect_create();
void connect_delete(Connect_t *con);


  sem_t sem_socket;
  int worker=0;
  int workerpid[2];
  int multiprocess=1;

int main(int argc,char *argv[]){

    hander_configure();

    int serverfd=bind_server("127.0.0.1",8080);


    if(sem_init(&sem_socket,1,1)<0) perror("Sem");

    if(multiprocess==1) {
        //多线程模式
        int i = 0;
        for (; i < 2; i++) {
            int pid = 0;
            if ((pid = fork()) == 0) {
                worker = 1;
                break;
            } else {
                workerpid[i] = pid;
            }
        }
        if (worker == 0) {
            printf("%d  I get two child %d %d\n", getpid(), workerpid[0], workerpid[1]);
        } else {
            printf("I am child %d \n", getpid());
            //    register_signal();
        }
    }

    main_loop(serverfd);//主循环

    return 0;
}

int bind_server(char *addr, int port){

    int server_fd=0;
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //inet_pton(AF_INET,addr,&server_addr.sin_addr);
    server_addr.sin_port=htons(port);

    server_fd=socket(PF_INET,SOCK_STREAM,0);
    int reuse=1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

    if(server_fd<=0){
        perror("Socket:");
        return 1;
    }

    int ret=0;
    ret=bind(server_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(ret<0){
        perror("Bind:");
        return 2;
    }

    ret=listen(server_fd,MAX_LISTEN);
    if(ret<0){
        perror("Listen:");
        return 3;
    }

    printf("--------Server is listening on %s %d  => pid:%d  listening on fd:%d\n",addr,port,getpid(),server_fd);

    return server_fd;
}

void hander_configure(){

}
void main_loop(int serverfd){

    int epollfd=epoll_create1(0);


    register_event(epollfd,serverfd,EPOLL_CTL_ADD,EPOLLIN);

    struct epoll_event events[MAX_EVENT];
    int number=0;
    int ret=0;

    while(1){

        ret=epoll_wait(epollfd,&events,MAX_EVENT,2);
        if(ret<0){
            perror("Epoll wait");
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
                        printf("process %d get clent %d  ",getpid(),clientfd);

                        Connect_t *con=connect_create();
                        con->fd=clientfd;//con->addr=client_addr;


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
                        continue;
                    }else{
          //              printf("Receive %d bytes\n",len);
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

void signal_hander(int sig){

    switch(sig){
        case SIGTERM:{

        };
        case SIGHUP:{

        };
        case SIGALRM:{

        };
        case SIGCHLD:{

        };
        case SIGQUIT:{

        };
        case SIGKILL:{

        }
        default:{
            if(worker==0) {
                kill(workerpid[0], SIGTERM);
                kill(workerpid[1], SIGTERM);
            }
            printf("You are terminated by signal %d",sig);
            exit(sig);
        }
    }
}
void register_signal(){
    signal(SIGCHLD,signal_hander);
    signal(SIGALRM,signal_hander);
    signal(SIGHUP,signal_hander);
    signal(SIGTERM,signal_hander);
    signal(SIGQUIT,signal_hander);
    signal(SIGKILL,signal_hander);
    signal(SIGSEGV,signal_hander);
}

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
int setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

Connect_t *connect_create(){
    Connect_t * con=(Connect_t*)calloc(1,sizeof(Connect_t));
    con->read_buf=(char*)malloc(2048);
    con->request=request_create();
    return con;
}

void connect_delete(Connect_t *con){
    request_delete(con->request);
    free(con->read_buf);
    free(con);
    con=NULL;
}