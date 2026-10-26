# `kernel/memory/memory.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../kernel/memory/memory.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:address`, `:range`, `:layout`, `:boot`, `:allocator`, `:sv39`, `:kernel_paging`, `:user_paging`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

该文件在清点范围内没有独立函数项；它提供模块重导出、类型、常量、宏或布局约束。不要把这些定义误当作可调用程序。

## 函数导航

本文件提供类型、常量、模块重导出或汇编入口；参见文件概览。
