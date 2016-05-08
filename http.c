//
// Created by think on 16-5-7.
//
#include "http.h"
#include <stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include <fcntl.h>

void http_handle(Request_t *req);
Request_t* request_create(){

    Request_t *request=(Request_t *)calloc(1,sizeof(Request_t));
    request->header=(Str_t*)calloc(20,sizeof(Str_t));
    return request;
}
void request_delete(Request_t* request){
    if(!request){
        free(request->header);
    }
    free(request);
}

Request_t* http_parse(char* content,int sfd){

    char *ptr=content;
    Request_t *request=request_create();
    request->fd=sfd;

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
        perror("Unknown");
        request->stat_code=4;
        return NULL;
    }

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

  //  printStr2(request->uri);

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
    //        printStr2(request->header[request->size]);
    //        write(STDOUT_FILENO,":",1);
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

    http_handle(request);
    response(request);
}
void http_handle(Request_t *req){
    char path[256]={'\0'};
    strncpy(path,"/home/think/html",16);
    strncpy(path+16,req->uri.str,req->uri.size);
    //uri初始化路径

    int idx=strchr(path,'?');
    if(idx==0){
        printf("no paragram\n");
    }else{
        path[idx]='\0';
    }
    int len=strlen(path);

    if(path[len-1]=='/'){
        strncat(path,"index.html",10);
    }

    printf("%s\n",path);

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

    req->file_fd=fd;
    req->stat_code=2;
    req->file_size=file_state.st_size;
}
void response(Request_t *req){

    switch(req->stat_code){
        case 2:{
            char *s="HTTP/1.1 200 OK\r\n\r\n";
           write(req->fd,s,strlen(s));
            sendfile(req->fd,req->file_fd,NULL,req->file_size);
        };break;
        case 4:{
            char *s="HTTP/1.1 400 BAD REQUEST\r\n\r\n";
            write(req->fd,s,strlen(s));
        };break;
    }

}

