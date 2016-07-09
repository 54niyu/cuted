//
// Created by think on 16-5-7.
//
#include "http.h"
#include "server.h"
#include <stdlib.h>
#include<sys/stat.h>
#include <fcntl.h>

void http_handle(Connect_t *con);
Request_t* request_create(){
    Request_t *request=(Request_t *)calloc(1,sizeof(Request_t));
    request->header=(Str_t*)calloc(20,sizeof(Str_t));
    return request;
}
void request_delete(Request_t* request){
    if(request->header!=NULL) free(request->header);
    free(request);
}

Request_t* http_parse(Connect_t *con){
    char *ptr=con->read_buf;
    Request_t *request = con->request;
    if(*ptr=='G'){
       if(strncmp(ptr,"GET",3)==0)
           ptr+=3;
    }else if(*ptr=='P'){
        if(strncmp(ptr,"POST",4)==0){
            ptr+=4;
            request->method=1;
        }
    }else if(*ptr=='H'){
        if(strncmp(ptr,"HEAD",4)==0){
            ptr+=4;
            request->method=2;
        }
    }else if(*ptr=='P'){
        if(strncmp(ptr,"PUT",3)==0){
            ptr+=3;
            request->method=3;
        }
    }else{
        printf("Unkown methon\n");
        request->stat_code=4;
        return NULL;
    }

    printf("Methon %d  ",request->method);

    while(*ptr==' ')
        ptr++;

    if(*ptr=='/'){
        request->uri.str=ptr;
        request->uri.size=0;
        while(*ptr!=' '){
            (request->uri.size)++;
            ptr++;
        }
    }else{
        request->stat_code=4;
       perror("URI"); return NULL;
    }

   // printStr2(request->uri);

    while(*ptr==' ')
        ptr++;

    if(strncmp(ptr,"HTTP",4)==0){
        request->version.str=ptr;
        ptr+=4;
        request->version.size=4;
        while(*ptr!='\r'){
            (request->version.size)++;
            ptr++;
        }
        if(*(++ptr)=='\n'){
            ptr++;
        }else{
            request->stat_code=4;
            perror("Line\n");return NULL;
        }
    }
    else{
        request->stat_code=4;
        perror("Unkown Http version\n");return NULL;
    }

   // printStr2(request->version);

    while(*ptr!='\0'){
        if(*ptr=='\r'&&*(ptr+1)=='\n'){
           ptr+=2;
            break;
        }else{
            request->header[request->size].str=ptr;
            request->header[request->size].size=0;
            while(*ptr!=':'){
                ptr++;request->header[request->size].size++;
            }

            ptr++;
            request->size++;
            request->header[request->size].str=ptr;
            request->header[request->size].size=0;
            while(*ptr!='\r'){
                ptr++;request->header[request->size].size++;
            }
            request->size++;
            if(*(++ptr)=='\n')
                ptr++;
        }
    }
    request->body.str=ptr;
    request->size=0;
    while(*ptr!='\0'){
       ptr++;request->size++;
    }

    http_handle(con);

}
void http_handle(Connect_t *con){

    Request_t *req=con->request;
    char path[256]={'\0'};
    char *base="/Users/Bing/AmazeUI-2.7.0";
    strncpy(path,base, strlen(base));
    strncpy(path+strlen(base),req->uri.str,req->uri.size);
    //uri初始化路径

    int idx=strchr(path,'?');
    if(idx==0){
 //       printf("no paragram\n");
    }else{
        path[idx]='\0';
    }
    int len=strlen(path);

    if(path[len-1]=='/'){
        strncat(path,"index.html",10);
    }

//    printf("%s\n",path);

    struct stat file_state;
    int ret=stat(path,&file_state);

    if(ret!=0){
        req->stat_code=4;
        return ;
    }

    int fd=open(path,O_RDONLY);
    if(fd<0){
        req->stat_code=4;
        return;
    }

    int nb= read(fd,con->write_buf,10240);

    con->write_buf[nb]='\0';
    req->file_size=nb;


    req->stat_code=2;
}
void response(Connect_t *con){

    Request_t *req=con->request;

    switch(req->stat_code){
        case 2:{
            char *s="HTTP/1.1 200 OK\r\n\r\n";
            write(req->fd,s,strlen(s));
            //sendfile(req->fd,req->file_fd,NULL,req->file_size);
            write(req->fd,con->write_buf,req->file_size);
            close(req->file_fd);
        };break;
        case 4:{
            char *s="HTTP/1.1 400 BAD REQUEST\r\n\r\n";
            write(req->fd,s,strlen(s));
        };break;
    }
    printf("Handle over\n");

}

