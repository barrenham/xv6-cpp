# `kernel/process/signals/signal_abi.h`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/signal_abi.h)。文档不会加入源码或生成额外 C++ 声明。

该文件在清点范围内没有独立函数项；它提供模块重导出、类型、常量、宏或布局约束。不要把这些定义误当作可调用程序。


## 格式、边界与使用约定

定义用户处理函数、屏蔽字和 sigaction 标志布局。SIGKILL/SIGSTOP 不可捕获或屏蔽；SIGCHLD 的默认忽略不等于本项目实现了 Linux 的显式 SIG_IGN 自动回收。
