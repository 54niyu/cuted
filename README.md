# CuteHttpd

    A light http server

## Include

* event base on epoll/kqueue
* containers map slice
* 统一事件源
* support for mac Os /Linux

## TODO

* 主从支持
* 静态服务
* 简单代理服务
* http 完善解析
* CGI / WSGI 支持

### 
* 基本思想是  
* io signal timer 抽象成 event
* 注册到不同的分发机制   epoll kqueue
* 当该事件发生时候，再将该 ev 放入 active_events 中 
