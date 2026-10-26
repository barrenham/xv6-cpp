# `kernel/boot/entry.S`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/entry.S)。文档不会加入源码或生成额外 C++ 声明。


<a id="asm-_entry"></a>

## `_entry`

```doxygen
@fn void _entry()
@brief 固件进入内核时为每个 hart 建立独立启动栈，再交给 C ABI 入口。
@details 入口位于 .text.entry。以 mhartid 得到 a0，令 sp=stack0+(hartid+1)*4096；保留固件 a1 中的设备树地址并 call start。start 若意外返回则落入 spin 死循环。
@pre 固件已把内核装入链接地址，stack0 提供每 hart 的启动栈空间。这里还没有普通进程 Context。
@param a0 由 mhartid 建立的 hart 编号。
@param a1 固件提供并保留的设备树地址。
@return 不返回普通调用者，后续特权级设置由 start/boot::enter 接手。
```

## 汇编入口与共享 ABI

- [`_entry`](entry.S.md#asm-_entry)
