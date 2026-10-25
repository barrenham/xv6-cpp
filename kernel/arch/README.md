# RISC-V 架构模块 xv.arch

内核 C++ 通过 `import xv.arch;` 调用 `xv::arch::r_*` / `w_*` 寄存器访问、
`intr_on/intr_off/intr_get`、`sfence_vma` 和 I/O / 指令缓存屏障。
这些 inline 函数属于具名模块，不依赖旧的全局头文件函数。

`riscv/register_operations.inc` 是共享指令定义：内核在 `xv::arch` 的导出
命名空间内编译它，用户程序通过 `riscv.h` 在自己的地址空间编译同一份代码。
`register_constants.h` 保留汇编与用户 ABI 使用的常量、页表编码宏及基础别名。
汇编直接包含常量头，内核 C++ 不再包含 `riscv.h`。

中断嵌套状态属于每核进程状态；禁用/恢复嵌套中断的服务由 `xv.sync` 提供，
不放进无状态的寄存器访问模块。
