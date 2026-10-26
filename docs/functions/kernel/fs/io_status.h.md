# `kernel/fs/io_status.h`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../kernel/fs/io_status.h)。文档不会加入源码或生成额外 C++ 声明。

该文件在清点范围内没有独立函数项；它提供模块重导出、类型、常量、宏或布局约束。不要把这些定义误当作可调用程序。


## 格式、边界与使用约定

定义暂不可用的直接负值返回约定。项目不维护 libc errno TLS；EWOULDBLOCK 与 EAGAIN 相同，不能把 -EAGAIN 当作 EOF。
