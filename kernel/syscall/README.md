# 系统调用模块 xv.syscall

陷阱平台实现通过 `import xv.syscall;` 和 `xv::syscall::dispatch()` 进入
系统调用分发。公开接口只包含 `dispatch()`；参数读取和具体处理函数均为
模块内部实现，不导出旧的全局 C 函数。

| 文件 | 职责 |
| --- | --- |
| `syscall.cppm` | 公开分发入口 |
| `syscall.cpp` | 从 a7 读取编号、选择处理函数、向 a0 写回结果 |
| `arguments.cppm` / `arguments.cpp` | 内部分区：读取 a0–a5 和用户地址、字符串 |
| `handlers.cppm` | 内部分区：统一声明处理函数的 64 位返回类型 |
| `sysproc.cpp` | 进程、时钟和页表实验处理函数 |
| `sysfile.cpp` | 文件系统、描述符、管道和 exec 参数处理 |
| `sysvm.cpp` | 从 support/stats.cpp 迁入的页表实验处理函数 |
| `syscall.h` | 内核和用户程序共享的系统调用编号 ABI |

用户态仍通过原来的 ecall、寄存器约定和编号进入内核；未知编号仍报告错误，
向 a0 写入 -1。系统调用处理函数的实现属于 `xv.syscall`，分发表直接引用
这些模块函数。`defs.h` 中的旧 syscall/arg*/fetch* 声明已删除。

页表处理函数通过 `xv.memory` 调用页表服务，填充与数组长度通过
`xv.runtime` 访问，寄存器和 TLB 操作导入 `xv.arch`，诊断导入 `xv.print`。
陷阱入口推进用户 PC、检查 killed、开中断等职责仍由 `xv.trap` 管理。
