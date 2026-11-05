# POSIX 消息队列

公共接口位于 `<mqueue.h>`。`mq_open("/name", flags, mode, &attr)` 创建或打开
独立于文件系统路径的名称；不使用 O_CREAT 时只传名称和 flags。不同进程可在
各自启动后打开同一名称，不要求共同继承描述符。接口返回 -1 并设置 errno。

```cpp
mq_attr attr{};
attr.mq_maxmsg = 16;
attr.mq_msgsize = 128;
mqd_t queue = mq_open("/jobs", O_CREAT | O_RDWR, 0600, &attr);
mq_send(queue, "job", 3, 5);
char message[128];
unsigned priority;
ssize_t length = mq_receive(queue, message, sizeof(message), &priority);
mq_close(queue);
mq_unlink("/jobs");
```

当前支持 open/close/unlink、send/receive、timedsend/timedreceive、getattr/setattr。
优先级范围 0..32767，数值大者先接收，同优先级按发送顺序接收。零长度消息有效。
接收缓冲区必须至少为配置的 mq_msgsize，消息不截断。mq_setattr 只修改 O_NONBLOCK，
不改变容量或消息大小。独立 open 的 flags 各自独立，dup/fork 共享同一打开描述。
描述符默认设置 FD_CLOEXEC；支持 poll，队列有消息时可读，有空位时可写。

不带 O_NONBLOCK 时，队列空/满的接收/发送登记等待通道并睡眠；改变条件后唤醒。
O_NONBLOCK 返回 EAGAIN。定时接口接收 CLOCK_REALTIME 的绝对 timespec，使用
clock_gettime 获取当前时刻；纳秒必须在 0..999999999，超时返回 ETIMEDOUT。
到期时间不会阻止一次可以立即完成的操作。收到可处理信号可返回 EINTR。
内核将绝对时间转换到单调截止时间队列，等待期间没有循环轮询。

名称形式为 /name，不能含中间斜杠。创建时应用 uid/gid、mode 和 umask；open 检查
读写权限。unlink 移除名称，已打开的描述符和消息继续有效，最后关闭后回收。
同名重新创建得到独立队列。名称最多 32 个，每队列最多 64 条消息、单条最多
3072 字节，默认 16 条/3072 字节。消息按需占用一页，不预分配全部容量。
资源限制为本系统选择，不是 POSIX 对所有系统规定的固定值。

尚未提供 mq_notify 异步通知，不声称完整 POSIX 实时扩展认证。
`/usr/tests/mqtest` 覆盖优先级/FIFO、权限、独立 exec 通信、满/空队列、poll、
双向阻塞唤醒、绝对超时、信号中断、错误用户指针、unlink/重建及描述符生命周期。
测试接入 `posix_ipc_{1,5}cpu`，结果以实际回归日志为准。
