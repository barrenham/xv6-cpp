# 调度目录

公开 Scheduler 接口、共用执行机制、策略入口与底层队列分开组织：

```text
scheduler/
├── scheduler.cppm                      公开 Scheduler 与 SchedulerKind
├── policies/
│   ├── fair.cppm                       FairPolicy，固定为公平策略
│   ├── round_robin.cppm                RoundRobinPolicy，固定为原 FIFO 轮转
│   ├── mlfq.cppm                       MlfqPolicy，固定为多级反馈策略
│   └── scheduling_policy.h             共享纯算法模型与运行时间记账
├── queues/
│   ├── ready_queue.cppm / .cpp         队列契约、进程链接与亲和性筛选
│   ├── fifo_ready_queue.cppm / .cpp    FIFO 实现
│   └── policy_ready_queue.cppm / .cpp  纯策略模型与 Process/锁/时钟的适配
└── runtime/
    ├── scheduler_runtime.cppm / .cpp   SchedulerRuntime，共用运行循环和策略委托
    └── scheduler_context.cppm / .cpp   状态、中断、进程锁、swtch 与 TLB 交接
```

SchedulerRuntime 接收 ReadyQueue，运行选取、派发、通知与空闲循环；
FairPolicy 和 MlfqPolicy 分别固定 PolicyReadyQueue 的模式，
RoundRobinPolicy 使用原 FifoReadyQueue。三个策略入口各有独立模块，
共同的算法与记账代码仍由 scheduling_policy.h 复用。
SchedulerContext 负责真正的进程状态转换及汇编切换。

服务层在 ../lifecycle/service.cpp 静态构造三种策略和一个运行机制，
启动时通过 initialize(SchedulerKind) 选择，默认 fair；不做运行时热切换。
外部仍通过 import xv.process 和 scheduler() 访问服务。

所有 hart 共用所选策略的就绪结构；每个 hart 的运行循环上下文、CpuState 和调度栈独立。
队列锁串行领取任务，队列锁释放后再获取进程锁；进程锁跨 swtch 交接，避免重复运行和锁环。
CPU 亲和性在出队时筛选。公平基准同时观察其他 hart 上正在运行的实体；
MLFQ 的提升在同一队列锁内覆盖排队和运行实体。
当前没有逐核运行队列、工作窃取或跨核唤醒 IPI；空闲 hart 由本核时钟中断重新检查工作。

每核对象与查询见 ../arch/cpu.cppm/.cpp，等待协议见 ../wait/，
首次进入用户态见 ../lifecycle/first_return.cpp。
算法、时间额度、锁协议和测试范围见 [调度设计](../../../docs/scheduling.md)。
