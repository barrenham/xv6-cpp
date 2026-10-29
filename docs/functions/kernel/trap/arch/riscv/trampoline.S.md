# `kernel/trap/arch/riscv/trampoline.S`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../../kernel/trap/arch/riscv/trampoline.S)。文档不会加入源码或生成额外 C++ 声明。

## 函数导航



本文件提供类型、常量、模块重导出或汇编入口；参见文件概览。

<a id="asm-uservec"></a>

## `uservec`

```doxygen
@fn void uservec()
@brief 从用户态陷阱入口保存用户寄存器并切换到内核页表和内核栈。
@details 使用 sscratch 暂存用户 a0，按 TrapFrame 的固定偏移保存其余用户寄存器，再把用户 a0 写入现场。从 trapframe 读取 kernel_sp、kernel_hartid、kernel_satp 与 kernel_trap，完成页表切换及地址翻译屏障后跳到内核入口。
@pre 用户页表和内核页表把 trampoline 映射到相同虚拟地址；用户页表中固定的 TRAPFRAME 映射必须有效且不允许用户直接访问。
@note 原 PC 的处理由 usertrap C++ 路径读取 sepc 完成。此标签不是可按普通 C ABI 调用的函数，文档原型仅表示控制流入口。
@return 不 ret；通过内核入口继续处理本次用户陷阱。
```

<a id="asm-userret"></a>

## `userret`

```doxygen
@fn void userret()
@brief 安装用户页表并恢复完整用户整数寄存器后返回用户态。
@param a0 用户 satp token；调用前 C++ 返回准备已经设置好 sepc 与 sstatus。
@details 按汇编要求刷新翻译并切换 satp，从固定 TRAPFRAME 位置恢复寄存器，最后恢复用户 a0 并执行 sret。汇编恢复顺序依赖 TrapFrame 偏移，不得只修改 C++ 结构而不调整汇编。
@pre stvec、kernel_* 返回字段、用户 PC 和特权状态已由 prepare_user_return 完成设置，且 trampoline 在切换前后均可执行。
@note trampoline 基址是共享映射标记而不是普通函数；本标签也不是通过普通函数返回协议离开的入口。
@return 通过 sret 进入用户态，不回到当前内核 C++ 栈帧。
```

## 汇编入口与共享 ABI

- [`uservec`](trampoline.S.md#asm-uservec)
- [`userret`](trampoline.S.md#asm-userret)
