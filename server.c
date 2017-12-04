//
// Created by think on 16-5-6.
//
//
#include "http.h"
#include "server.h"
#include "event.h"


#define MAX_LISTEN 128
#define MAX_EVENT 128

void handle_configure();//配置处理
int bind_server(char *addr, int port);//初始化参数，bind服务器
void main_loop(int serverfd);//主循环


void read_client(int, void *);
void write_client(int, void *);
void accept_client(int, void *);
void handle_signal(int, void *);
void main_loop2(int);

int worker = 0;
int workerpid[2] = {0};
int multiprocess = 0;
int run = 0;

int set_nonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char *argv[]) {

    argv[0] = NULL;

    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    handle_configure();

    int serverfd = bind_server("127.0.0.1", 9905);

    //    register_signal();//注册事件处理
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

    main_loop(serverfd);//主循环

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

void accept_client(int fd, void *arg) {

    ssize_t socklen;
    struct sockaddr client_addr;

    int client = accept(fd, &client_addr, &socklen);
    if (client < 0) {
        perror("Accept");
        return;
    }
    printf("process %d accept client %d  \n", getpid(), client);
    set_nonblocking(client);
    //    struct kevent chg;
    connect_t *con = connect_create();
    con->fd = client;

    event_t *ev = new_event(client, 1, CUTE_READ, read_client, con);
    Reactor *rc = (Reactor *) arg;
    con->backend = rc;
    reactor_add(rc, ev, NULL);
}

void read_client(int fd, void *arg) {

    printf("reading from %d..\n", fd);
    connect_t *con = arg;

    int len = read(con->fd, con->read_buf, 1024);
    if (len <= 0) {
        perror("Read");
        if (errno != EAGAIN){
            event_t *ev = new_event(fd, 1, CUTE_READ, write_client, con);
            reactor_del((Reactor *) con->backend, ev);
            close(fd);
        }
        return;
    } else {
        printf("Receive %d bytes\n ", len);
    }
    con->read_buf[len] = '\0';
    con->request->fd = fd;

    http_parse(con);
    event_t *ev = new_event(fd, 1, CUTE_READ, write_client, con);
    reactor_del((Reactor *) con->backend, ev);

    ev = new_event(fd, 1, CUTE_WRITE, write_client, con);
    reactor_add((Reactor *) con->backend, ev, NULL);
}

void write_client(int fd, void *arg) {

    //处理写事件
    response(arg);
    //删除监听事件
    connect_delete(arg);
    close(fd);//关闭连接
}

void handle_timeout(int fd, void* arg){
    static int count = 0;
    printf("Halo\n");
    count++;
    if( count > 10){
        event_t *ev = arg;
        reactor_del(ev->rc,ev);
    }
}


void handle_signal(int sig,void * data) {
    printf("Caught %d\n",sig);
}


void main_loop(int server_fd) {

    Reactor *rc = reactor_create();
    event_t *sev = new_event(server_fd, 1, CUTE_READ | CUTE_PERSIST, accept_client, rc);
    event_t  *sigev = new_event(SIGUSR1, CUTE_SIGNAL ,CUTE_SIGNAL | CUTE_PERSIST, handle_signal, rc);
    event_t *tev = new_event(-1, CUTE_TIMEOUT, CUTE_TIMEOUT|CUTE_PERSIST, handle_timeout , rc);
    tev->arg = tev;

    reactor_add(rc, sev,NULL);
    reactor_add(rc, sigev,NULL);
    struct timeval *tm = (struct timeval *)malloc(sizeof(struct timeval));
    tm->tv_sec = 5;
    tm->tv_usec = 0;
    reactor_add(rc, tev, tm);
    reactor_loop(rc);

}


