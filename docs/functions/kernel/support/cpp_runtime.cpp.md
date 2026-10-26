# `kernel/support/cpp_runtime.cpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/cpp_runtime.cpp)。文档不会加入源码或生成额外 C++ 声明。

直接模块依赖：`xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`__cxa_pure_virtual`](cpp_runtime.cpp.md#fn-f28d025dc990c879)

<a id="fn-f28d025dc990c879"></a>

## `__cxa_pure_virtual`

```doxygen
@fn void __cxa_pure_virtual()
@brief freestanding 运行时、字节操作和诊断输出：cxapurevirtual。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```
