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

`SYS_ioctl=40` 通过 `sysfile.cpp` 解析描述符、请求号和机器字宽参数，调用
`OpenFileTable::ioctl()`。只向字符设备转发，由设备验证请求和用户缓冲；
普通文件、管道、无效描述符及不支持的请求返回 -1。TTY 请求和结构布局见
`kernel/tty/termios.h`，用户态封装位于 `user/runtime/termios.cpp`。

进程身份系统调用编号为 getpgrp=41、setpgid=42、getsid=43、setsid=44。
`sysproc.cpp` 只读取参数并转发给 `xv.process`；组/会话查找、权限和资源回收
由 `ProcessGroups` 管理。用户声明与汇编入口分别位于 user.h 和 usys.S。

`sigsend=45` 显式指定目标与信号，支持 SIGINT、SIGKILL、SIGTERM，以及信号 0
探测；旧 kill(pid) 保持 ABI 和退出状态。错误返回 -1，具体选择和状态约定见
[最小信号说明](../process/signals/README.md)。

`sigdisposition=46` 设置 SIGINT/SIGTERM 的默认或忽略动作，返回旧动作。
TIOCSCTTY/TIOCGPGRP/TIOCSPGRP 通过原 ioctl 接口实现；用户态增加
`tcgetpgrp/tcsetpgrp`。更改和查询前台组受控制终端与会话关系约束。

`waitpid=47` 按 PID/进程组选择自己的子进程，支持 WNOHANG/WUNTRACED/WCONTINUED，
使用 wait.h 的状态编码。旧 wait 仍返回普通退出值。停止信号导致可重试 syscall
无传输返回 -1 时，分发器保留参数并回退 ecall，继续后重新执行，部分传输不重放。
