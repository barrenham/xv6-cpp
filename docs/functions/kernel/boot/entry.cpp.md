# `kernel/boot/entry.cpp`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/entry.cpp)。文档不会加入源码或生成额外 C++ 声明。

直接模块依赖：`xv.boot`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`start`](entry.cpp.md#fn-90bc9147889a0726)

<a id="fn-90bc9147889a0726"></a>

## `start`

```doxygen
@fn void start(unsigned long hart_id, unsigned long device_tree)
@brief 启动、每 hart 初始化和进入内核的交接：入口。
@param hart_id 当前启动 hart 的编号。
@param device_tree 固件传入的设备树物理地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::boot::enter`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
