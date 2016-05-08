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

#include "http.h"

#define MAX_LISTEN 128
#define MAX_EVENT 128

void hander_configure();//配置处理
int bind_server(char *addr, int port);//初始化参数，bind服务器
void main_loop(int epollfd,int serverfd);//主循环
void signal_hander(int sig);//信号处理函数
void register_signal();//信号注册
void register_event(int epollfd,int fd, unsigned int op, unsigned int e);//事件处理


int main(int argc,char *argv[]){

    hander_configure();

    int serverfd=bind_server("127.0.0.1",8080);

    register_signal();

    int epollfd=epoll_create1(0);

    register_event(epollfd,serverfd,EPOLL_CTL_ADD,EPOLLIN);

    main_loop(epollfd,serverfd);

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
void main_loop(int epollfd,int serverfd){

    struct epoll_event events[MAX_EVENT];
    int number=0;
    int ret=0;
    while(1){

        ret=epoll_wait(epollfd,&events,MAX_EVENT,0);
        if(ret<0){
            perror("Epoll wait");return;
        }

        int i=0;
        for(;i<ret;i++){
           int sfd=events[i].data.fd;
            if(sfd==serverfd){

                if(events[i].events&EPOLLIN){
                    if(number<MAX_EVENT) {
                        struct sockaddr_in client_addr;
                        socklen_t len = sizeof(client_addr);
                        int clientfd = accept(serverfd, (struct sockaddr *) &client_addr, &len);
                        if (clientfd < 0) {
                            perror("Accept");
                            continue;
                        }
                        number++;
                        register_event(epollfd, clientfd, EPOLL_CTL_ADD, EPOLLIN);
                    }
                }

            }else{

                if(events[i].events&EPOLLIN){
                    char buf[5120]={'\0'};
                    int num=recv(sfd,buf,5120,0);
                    if(num>0){
                        buf[num+1]='\0';
                        http_parse(buf,sfd);
                        //注册写监听事件
                        register_event(epollfd,events[i].data.fd,EPOLL_CTL_MOD,EPOLLOUT);
                    }
                }
                else if(events[i].events&EPOLLOUT){
                    //删除监听事件

                    register_event(epollfd,events[i].data.fd,EPOLL_CTL_DEL,EPOLLIN);
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
        default:{
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
}

void register_event(int epollfd,int fd, unsigned int op, unsigned int e){
    struct epoll_event event;
    bzero(&event,sizeof(event));
    event.data.fd=fd;
    event.events=e;
    epoll_ctl(epollfd,op,fd,&event);
    return;
}