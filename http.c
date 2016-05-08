//
// Created by think on 16-5-7.
//
#include "http.h"
#include <stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include <fcntl.h>

void http_handle(Request_t *req);

Request_t* http_parse(char* content,int fd){

    printf("%s \n",content);
    char *ptr=content;
    Request_t *request=(Request_t *)malloc(sizeof(Request_t));
    request->fd=fd;
    request->method=0;
    request->size=0;
    request->uri=(Str_t*)malloc(sizeof(Str_t));
    request->version=(Str_t*)malloc(sizeof(Str_t));
    request->header=(Str_t*)malloc(sizeof(Str_t)*20);

    if(*ptr=='G'){
       if(strncmp(ptr,"GET",3)==0)
           ptr+=3;
    }else if(*ptr=='P'){
        if(strncmp(ptr,"Post",4)==0){
            ptr+=4;
            request->method=1;
        }
    }else{
        perror("Method");return NULL;
    }


    while(*ptr==' ')
        ptr++;

    if(*ptr=='/'){
        request->uri->str=ptr;
        request->uri->size=0;
        while(*ptr!=' '){
            (request->uri->size)++;
            ptr++;
        }
    }else{
       perror("URI"); return NULL;
    }

    printStr1(request->uri);

    while(*ptr==' ')
        ptr++;

    if(strncmp(ptr,"HTTP",4)==0){
        request->version->str=ptr;
        ptr+=4;
        request->version->size=4;
        while(*ptr!='\r'){
            (request->version->size)++;
            ptr++;
        }
        if(*(++ptr)=='\n'){
            ptr++;
        }else{
            perror("Line\n");return NULL;
        }
    }
    else{
        perror("Unkown Http version\n");return NULL;
    }

    printStr1(request->version);

    while(*ptr!='\0'){
        if(*ptr=='\r'&&*(ptr+1)=='\n'){
           ptr+=2;
        }else{
            request->header[request->size].str=ptr;
            request->header[request->size].size=0;
            while(*ptr!=':'){
                ptr++;request->header[request->size].size++;
            }
            printStr2(request->header[request->size]);
            write(STDOUT_FILENO,":",1);
            ptr++;
            request->size++;
            request->header[request->size].str=ptr;
            request->header[request->size].size=0;
            while(*ptr!='\r'){
                ptr++;request->header[request->size].size++;
            }
            printStr2(request->header[request->size]);
            request->size++;
            printf("\n");
        }
    }

    http_handle(request);
}
void http_handle(Request_t *req){
    char path[256]={'\0'};
    strncpy(path,"/home/think/html",16);
    strncpy(path+16,req->uri->str,req->uri->size);
    char* ptr=path;
    int idx=strchr(path,'?');
    if(idx==0){
        printf("no get paragram\n");
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
        perror("File");
        write(req->fd,"HTTP/1.1 503 BAD REQUEST\r\n\r\n",200);
        return ;
    }

    int fd=open(path,O_RDONLY);
    if(fd<0){
        perror("Open file");
        return;
    }

    char *res="HTTP/1.1 200 OK\r\ncontent-length:5120\r\n\r\n";
    write(req->fd,res,strlen(res));
    sendfile(req->fd,fd,NULL,file_state.st_size);
}
char *response(Request_t *req){

}

