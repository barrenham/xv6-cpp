# 非阻塞文件状态与 fcntl

共享标志在 fcntl.h，系统调用 fcntl=50。当前支持 F_GETFL 和 F_SETFL，
用户声明为变参接口，F_GETFL 不需要第三个参数：

```cpp
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"

int flags = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
int n = read(fd, buffer, sizeof(buffer));
if (n == -EAGAIN) {
  // 等待下一次 poll 就绪，再重试；不把它当作 EOF。
}
```

open(path, mode | O_NONBLOCK) 也可直接设置。F_GETFL 返回访问模式和状态标志，
不返回 O_CREATE/O_TRUNC。F_SETFL 只修改 O_NONBLOCK，忽略访问模式位的修改，
拒绝未知状态位与命令且不改变原值。访问模式和原有系统调用编号保留。

## 返回约定

| 结果 | 返回值 |
| --- | --- |
| 暂无数据、空间或内部读写锁被占用 | -EAGAIN（-11） |
| 已完成部分传输 | 实际输入字节数量，调用者保留剩余部分 |
| EOF / 挂断后输入排空 | 0 |
| 原有访问、复制、关闭对端等错误 | -1 |

本项目没有 errno 线程局部变量或 libc 错误转换，read/write 直接返回负值。
EWOULDBLOCK 与 EAGAIN 相同。零长度有效读写返回 0，不等待数据或锁。
这里没有引入完整 POSIX errno、SIGPIPE、F_DUPFD 或关闭时执行标志。

## 状态所有权

O_NONBLOCK 保存在 File（打开文件描述）中，用原子读写更新。dup/fork 共享
File，也共享标志；分别 open 同一个设备会得到独立标志。终端 termios 仍归
Tty 所有，因此同一设备的这些独立 File 仍共用终端设置。每次读写取得一次
状态快照，F_SETFL 不承诺取消已经进入等待的阻塞操作。

## 各对象的行为

- 管道：空且仍有写端时 read 返回 EAGAIN；无写端且排空后返回 0。写端无
  读者时返回原有错误 -1。长度不超过 512 的写操作等待/检查足够的整体空间，
  非阻塞时空间不足不写入任何字节；更长请求允许部分写入，完全没有空间才
  返回 EAGAIN。用户缓冲出错仍可能产生已复制部分的短写。
- TTY/slave：规范模式等到可提交记录，否则返回 EAGAIN；VEOF 记录照常返回
  一次 0。非规范模式立即读取已有数据，忽略 VMIN/VTIME 等待条件。即使
  VMIN=0/VTIME=0，非阻塞空输入也选择返回 EAGAIN，避免与 EOF 混淆。
- PTY master：读取已有输出，空且 slave 未关闭时返回 EAGAIN。输入缓冲满
  或输入串行锁忙时返回部分写入量/EAGAIN；输入仍经过行规程。原有规范长行
  截断规则保留。关闭对端后采用原有 EOF/错误约定，不返回伪 EAGAIN。
- 普通文件：接受和保存 O_NONBLOCK，但文件系统和存储等待仍可阻塞，与这类
  标志主要用于流设备的约定一致。ioctl 的输出排空也不改成非阻塞操作。

SleepLock::try_acquire 不登记睡眠，用于非阻塞读写的读者、写者和 UART
发送锁。poll 同时订阅锁释放通道与数据/空间通道，避免只观察缓冲而漏掉
锁恢复可用的通知。状态自旋锁仍用于短临界区，非阻塞不代表无锁。

## 输出转换与 UART

TTY 的非阻塞写按输入字符提交后端，一个字符经 OPOST/ONLCR 可能形成
两个字节。后端必须完整接受这个单元或返回错误，避免写出 CR 后又以相同
输入重试，产生重复的 CRLF。返回数量始终按原始输入字节计算。

UART 新增 128 字节发送队列：进程把完整转换单元入队，TX 中断继续发送；
没有空间时非阻塞调用返回 EAGAIN，阻塞调用可中断等待。队列状态由自旋锁
保护，发送顺序由发送锁串行化。drain 等待队列和硬件移位寄存器排空。
同步回显和内核日志先按序排空已有队列；panic 路径绕过发送锁以免递归死锁。
PTY 的 1024 字节输出队列沿用相同的完整转换单元规则。

## 代理与 shell

pty 代理对 stdin/stdout/master 开启非阻塞，通过 poll 批量读写 256 字节缓冲。
短写只推进对应游标，EAGAIN 保留缓冲并等待下一事件；挂断后先排空输出。
它在正常清理时恢复 stdin/stdout 原标志，并要求在前台启动，避免后台程序
先修改 shell 共享的标志和 termios。slave 上运行的内层程序保持阻塞描述符。

shell 在释放启动门之前保存终端文件标志；前台作业停止或结束后恢复，fg
时再安装被暂停作业的标志。这样代理或测试程序被 Ctrl-Z、Ctrl-C、SIGKILL
中断后，shell 不会因为继承 O_NONBLOCK 把 EAGAIN 当作输入结束。

## 验证

ptytest nonblock 检查 open/fcntl、dup/fork 共享、独立打开、不可变访问模式、
空/满管道、512 字节原子写、大块部分传输、规范 EOF、VMIN/VTIME 覆盖、
CRLF 完整性、端点挂断、后台代理拒绝；nonblock_1cpu/nonblock_3cpu 还检查
作业停止/继续和异常退出后 shell 标志恢复。原生检查覆盖锁竞争、空输入
不睡眠、UART 队列满及 TX 中断排空。PTY 交互中 cat README 完整转发 2441
字节，覆盖多个代理缓冲和 UART 部分写入。此次不运行完整回归。
