# `kernel/trap/trap.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../kernel/trap/trap.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:event`, `:dispatch`, `:clock`, `:controller`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::initialize`](service.cpp.md#fn-d8b22eb1636b2b5c)
- [`xv::trap::initialize_hart`](service.cpp.md#fn-a54f903e3675d4e8)
- [`xv::trap::prepare_return`](service.cpp.md#fn-0968469d414ee0fc)
- [`xv::trap::return_to_user`](service.cpp.md#fn-3733f8d54bf76c4c)

