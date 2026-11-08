# 内核线程与 pthread

实现一对一内核线程和基础 POSIX 风格 pthread 库。每个进程最多 32 个执行槽位，
包含初始线程以及尚未 join 回收的线程。应用默认链接线程运行时。

## 线程和进程的关系

线程共享地址空间、文件描述符表、工作目录、凭据和进程 PID；每个线程有独立
TID、寄存器、用户栈、内核栈、陷阱帧、信号 mask、TLS 和 errno。调度器独立
调度每个线程，可在多个 CPU 上执行。陷阱入口通过 sscratch 找到当前陷阱帧；
额外陷阱帧映射只允许 supervisor 访问。内核锁的持有者标识使用 TID。

pthread_exit 只退出调用线程；初始线程提前退出后，其他线程可以继续工作。
最后一个线程关闭共享资源，父进程才收到进程退出事件。exit 或 main 返回会
终止整组线程。join 等待目标切出内核栈后回收其用户栈、TLS 和陷阱帧。

fork 只复制调用线程到子进程，复制地址空间、FD 和信号处置；运行时修复子进程
线程登记表。exec 完整验证候选 ELF 后取得唯一执行者身份，阻止线程发布，
终止并回收其他线程，再提交新地址空间。工作线程也可以 exec，进程 PID 保持。
失败的 ELF 加载保留其他线程和原程序。

信号处置由整个进程共享，mask 和线程定向 pending 独立。进程定向信号保存在
共享 pending 集合，由一个未屏蔽的活跃线程领取；被选线程在实际进入处理器前
退出时，尚未送达的进程信号重新排队。pthread_kill 的信号不迁移到其他线程。
默认停止/继续作用于整组，父进程只收到稳定 PID 的一次事件，组首先退出时也适用。
sigaction 的动作快照和更新使用代次重试，用户复制期间不持信号表自旋锁。

## 用户接口

头文件：user/include/pthread.h。实现按职责分布在
user/runtime/threads/{tls,pthread,sync}.cpp，内核线程生命周期位于
kernel/process/lifecycle/threads.cpp。

提供 pthread_create/join/exit/self/equal/kill，栈大小属性，普通/递归/错误检查
mutex 和 condition wait/signal/broadcast。pthread 接口返回正的错误码，
不通过 errno 返回错误。支持静态 mutex/condition 初始化。

竞争等待使用按进程资源和地址索引的内核等待队列；比较数值和登记等待在同一
队列锁下完成，避免丢失唤醒。malloc/free 保护共享空闲链表。ELF 的 .tdata/.tbss
初始化每个线程的静态 TLS；tp 指向 TLS 数据区，errno 是线程局部变量。
栈默认 64 KiB，可设置 16..512 KiB，并带不可访问保护页。

```cpp
#include <pthread.h>

void *worker(void *argument) {
  return argument;
}

// 在应用 main 中：
// pthread_t thread;
// int error = pthread_create(&thread, nullptr, worker, nullptr);
// if (!error) pthread_join(thread, nullptr);
```

## 并发内存与 I/O

地址空间使用短时递归页表锁；后端加载、同步和最后释放在锁外执行，恢复后
重新检查映射代次、权限、EOF 和 PTE。共享执行时修改映射先阻止用户返回，
在锁外等待各 hart 离开旧映射，再修改或释放页面。后台回收只尝试锁。
sbrk 的大小调整和旧地址返回在同一地址空间操作内完成。

FD 查询在表锁内 retain，系统调用按线程持有临时引用，避免并发 close 释放
正在使用的对象。每次 FD 绑定有独立代次，失败回滚只撤销本次安装；open 在
发布 FD 前完成文件初始化。fork 同步取得 FD 和 CLOEXEC 快照。

管道、TTY、PTY、消息队列、本地 socket、共享内存及键盘/随机数驱动在自旋锁外
复制用户内存，让复制引起的 COW 缺页可以安全等待。读端使用预留、对象引用或
睡眠锁保持数据有效；复制失败保留未消费的数据。TTY flush 和键盘溢出使用
代次识别已替换队列，避免复制完成后误消费新数据。

## 验证

客体可执行：

- /usr/tests/threadcore：共享资源、独立 TID/栈、地址等待、反复创建回收、主线程退出。
- /usr/tests/pthreadtest：TLS/errno、mutex/condition、并发分配、join、信号共享和路由。
- /usr/tests/threadexec：失败 exec 保留兄弟线程、主/工作线程替换、竞争 exec 与 create。
- /usr/tests/threadstop：整组停止/继续、一次父进程事件、停止状态下终止整组。
- /usr/tests/threadio：fork 后多类并发 I/O 的 COW、子进程私有快照、工作线程 fork/exec、
  多线程 mmap/mprotect/munmap 与页面回收。

测试使用独立磁盘副本、串行 QEMU；构建限制一个任务，降低对开发虚拟机的影响。
CMake 测试入口为 thread_core_1cpu/5cpu 和 pthread_runtime_1cpu/5cpu，后者执行
pthreadtest、threadexec、threadstop、threadio，退出后检查文件系统一致性。

阶段证据：

- build/threads-runtime-integrated-validation.log：17/17 底层、驱动、IPC、VM、FS 和线程回归。
- build/threads-directed-signals-validation.log：7/7，包含单/五核信号、pthread 与线程定向投递。
- build/threads-group-control-validation.log：6/6，包含单/五核整组停止/继续。
- build/threads-pipe-validation.log：4/4，包含并发管道复制与工作线程 fork。
- build/threads-device-io-validation.log：5/5，驱动、五核信号/IPC、单/五核多类 COW I/O。

最终整合记录：

- build/threads-final-image-build.log：最新测试应用及镜像构建成功。
- build/threads-signal-exit-final-build.log：最终内核构建成功。
- build/threads-final-integrated-validation.log：16 项中 15 项通过，包含单/五核线程核心和
  pthread 扩展测试、五核驱动/IPC/VM/FS，以及底层进程契约。首次原生信号测试发现
  退出线程仍保留私有 pending 位，随后修正清理；pthread 扩展用例运行在
  修正后的内核，包含竞争 exec/create、随机数 COW 和并发映射变更。
- build/threads-signal-exit-native-validation.log：修正后原生信号契约通过。
- build/threads-final-sdk-validation.log：完整导出 SDK，在独立目录编译 ext4 ELF，
  验证存在 PT_TLS 段；无需内核实现源码。

build/threads-signal-exit-final-validation.log：修正后的原生信号与单/五核客体信号
兼容回归 3/3 通过。上述线程目标验收完成。
阶段记录各自对应当时构建，保留失败记录以便追溯。

## 当前接口边界

目前仅支持 joinable 线程、静态 TLS、未定时的 mutex/condition 等待。
没有 pthread_detach/cancel、TLS key/destructor 或实时调度。condition 已支持 timedwait，
mutex/join 暂未提供定时等待。
PROCESS_SHARED mutex/condition 明确返回 EOPNOTSUPP；不宣称完整 POSIX 认证。
多线程 fork 后，子进程在 exec 前只应使用异步信号安全接口，不承诺继承中的
malloc 或用户锁可继续使用。FS SDK 后续已统一迁移为线程池，详见文末链接。
独立 FS SDK 导出包包含 pthread/TLS 头文件、运行时和链接脚本。

本轮没有安装持久根盘、提交或推送；用户的 VS Code 配置保持原样。

## 后续 FS 迁移

本文件记录基础线程阶段；后续目标已将 SDK 的进程池统一迁移为 pthread 工作池，
并补充读写锁、条件变量超时等待和 ps -T。当前范围及验证见
[统一 FS 线程池](fs-persistent-workers.md)，不再适用上文“本轮不迁移 FS 工作池”的阶段限制。
