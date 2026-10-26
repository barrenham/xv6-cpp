# `kernel/trap/arch/riscv/entry.h`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../../kernel/trap/arch/riscv/entry.h)。文档不会加入源码或生成额外 C++ 声明。

## 函数导航

- [`kerneltrap`](entry.cpp.md#fn-3307905e8431d793)
- [`kernelvec`](entry.h.md#fn-7a8c0e690743bc6f)
- [`usertrap`](entry.cpp.md#fn-e31b5236fa8fe95c)

<a id="fn-7a8c0e690743bc6f"></a>

## `kernelvec`

```doxygen
@fn void kernelvec()
@brief 陷阱快照、事件分派、设备服务及特权级返回：kernelvec。
@details 在当前内核栈上预留 256 字节，保存陷阱入口所需通用寄存器，调用 kerneltrap，再恢复寄存器并 sret。恢复时不装回旧 tp，以保留调度迁移后的当前 hart 标识。
@pre 已处于内核页表和有效内核栈。kerneltrap 要求来源为 supervisor，且进入时中断关闭。
@note sepc/sstatus 的跨调度恢复由 C++ TrapController/平台快照路径处理，不应把本例程当作普通用户态完整现场保存器。
@return 经 sret 返回原来的内核执行流。
```

## 格式、边界与使用约定

只声明跨 C/汇编边界的入口和 trampoline 标签。函数名的 C 链接约定必须与汇编符号一致；TrapFrame/Context 偏移还由各自结构的静态断言维护。
