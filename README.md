# CuteHttpd

    A light http server

## Include

* event base on epoll/kqueue/select
* containers map slice

## TODO

* 整合 io signal timer 事件源
* 主从支持
* 静态服务
* 简单代理服务
* http 完善解析
* CGI / WSGI 支持



### 
基本思想是  
io signal timer 抽象成 event
注册到不同的分发机制   epoll kqueue
当该事件发生时候，再将该 ev 放入 active_events 中 

