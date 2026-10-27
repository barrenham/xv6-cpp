# `kernel/trap/handlers/page_fault.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/handlers/page_fault.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:handlers`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::PageFaultHandler::accepts`](page_fault.cpp.md#fn-fd588082eee9ffc3)
- [`xv::trap::PageFaultHandler::handle`](page_fault.cpp.md#fn-b2145e70bef0bcc2)

<a id="fn-fd588082eee9ffc3"></a>

## `xv::trap::PageFaultHandler::accepts`

```doxygen
@fn bool xv::trap::PageFaultHandler::accepts(const TrapEvent &event) const
@brief 匹配用户态的读写和指令缺页。
@details 要求异常来源不是 supervisor，内核栈缺页由更早的应急汇编入口处理，不能进入用户匿名补页逻辑。
```

<a id="fn-b2145e70bef0bcc2"></a>

## `xv::trap::PageFaultHandler::handle`

```doxygen
@fn TrapAction xv::trap::PageFaultHandler::handle(const TrapEvent &event)
@brief 以明确访问类型请求用户缺页恢复。
@details 按原因传递 Read/Write/Execute，解析成功返回 Resume 以重试原 PC，未处理返回 Unhandled 交给控制器终止进程，不推进异常指令 PC。
```
