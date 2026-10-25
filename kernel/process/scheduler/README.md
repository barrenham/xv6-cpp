# 调度目录

此目录只包含调度接口、选择策略、就绪队列和执行机制。等待事件、每核对象和
首次进入用户态分别归入 process/wait、process/arch 和 process/lifecycle。

```text
scheduler/
├── scheduler.cppm                  公开 Scheduler 接口
├── policies/
│   ├── round_robin.cppm             内部 RoundRobinScheduler 声明
│   └── round_robin.cpp              从注入的 ReadyQueue 选择任务
├── queues/
│   ├── ready_queue.cppm / .cpp      就绪队列抽象与受保护的链接操作
│   └── fifo_ready_queue.cppm / .cpp FIFO 实现、重复入队保护、CPU 绑定筛选
└── runtime/
    └── scheduler_context.cppm / .cpp
                                    空闲、中断窗口、状态转换、锁与 swtch 交接
```

`RoundRobinScheduler` 不包含 CPU 数组，不管理 sleep，不初始化文件系统，
不计算硬件寄存器或直接访问 Process 私有状态。它选择任务，再交给
`SchedulerContext` 执行；挂起、yield 和就绪发布也复用这套机制。
新增调度策略可以使用其他 ReadyQueue 实现并复用相同的上下文切换协议。

相关实现：

| 路径 | 职责 |
| --- | --- |
| `../arch/cpu.cppm` / `cpu.cpp` | CPU 数组与 current_cpu/current/cpu_id 查询 |
| `../arch/cpu_affinity.cpp` | Process::pin_cpu 的范围检查及原子绑定 |
| `../wait/wait_channels.cppm` / `.cpp` | 按通道索引等待者，登记、取消和唤醒 |
| `../wait/sleep.cpp` | 当前进程的 prepare_sleep/sleep 协议 |
| `../lifecycle/first_return.cpp` | 首次调度后的文件系统初始化、加载 /init 和返回用户态 |
| `../lifecycle/service.cpp` | 装配存储、就绪队列、调度器和等待索引 |

模块分区名保持稳定，物理目录反映职责；公开使用方仍通过
`import xv.process;` 和 `scheduler()` 访问服务。
