# `kernel/sync/spin/spinlock.cpp`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/spin/spinlock.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::SpinLock::acquire`](spinlock.cpp.md#fn-802a415222da16f6)
- [`xv::sync::SpinLock::holding`](spinlock.cpp.md#fn-53ef4002e320bbcd)
- [`xv::sync::SpinLock::init`](spinlock.cpp.md#fn-2ed907681aa58c53)
- [`xv::sync::SpinLock::release`](spinlock.cpp.md#fn-c91cfd40f4e7bacf)

<a id="fn-2ed907681aa58c53"></a>

## `xv::sync::SpinLock::init`

```doxygen
@fn void xv::sync::SpinLock::init(const char *label)
@brief 自旋锁：init。
@param label 保存在对象中的调试标签，字符串需在使用期间有效。
@details 本函数可见的状态更新包括：`name_ = label;`；`locked_ = 0;`；`owner_ = nullptr;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-802a415222da16f6"></a>

## `xv::sync::SpinLock::acquire`

```doxygen
@fn void xv::sync::SpinLock::acquire()
@brief 自旋锁：取得锁所有权。
@details 先压低当前 hart 的中断使能并检查递归获取，再用 acquire 原子交换竞争 locked_，成功后记录 owner_。忙等阶段不能调度；禁中断嵌套由 release 的配对 pop 恢复。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::push_interrupts_off` → `holding` → `detail::fail` → `__atomic_exchange_n` → `__atomic_store_n` → `detail::current_cpu`。
@details 需要特别跟踪的条件：`holding()`；`__atomic_exchange_n(&locked_, 1, __ATOMIC_ACQUIRE) != 0`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c91cfd40f4e7bacf"></a>

## `xv::sync::SpinLock::release`

```doxygen
@fn void xv::sync::SpinLock::release()
@brief 自旋锁：释放当前引用或归还对象占用的资源。
@details 先验证 owner_ 是当前 CPU，再清空拥有者，以 release 存储发布受保护写入，最后弹出中断屏蔽嵌套。释放锁不等价于无条件打开中断。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`holding` → `detail::fail` → `__atomic_store_n` → `detail::pop_interrupts_off`。
@details 需要特别跟踪的条件：`!holding()`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-53ef4002e320bbcd"></a>

## `xv::sync::SpinLock::holding`

```doxygen
@fn bool xv::sync::SpinLock::holding() const
@brief 自旋锁：检查当前执行者是否拥有此锁。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `detail::current_cpu`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&locked_, __ATOMIC_RELAXED) != 0 && __atomic_load_n(&owner_, __ATOMIC_RELAXED) == detail::current_cpu()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ddedf097009cd32c"></a>

## `xv::sync::SpinLock::acquire_spins`

```doxygen
@fn unsigned long xv::sync::SpinLock::acquire_spins()
@brief 获取自旋锁并返回失败原子交换次数。
@details 自旋锁保持原先 pushOff/popOff、递归获取检查和 owner 协议。try_acquire 失败恢复中断嵌套，成功保留中断屏蔽至 release；acquire_spins 返回失败原子交换次数，不在无竞争路径读取时钟。原 acquire 调用者行为保留。
```

<a id="fn-1ed9206d4f9fc80e"></a>

## `xv::sync::SpinLock::try_acquire`

```doxygen
@fn bool xv::sync::SpinLock::try_acquire()
@brief 尝试一次获取；失败恢复中断嵌套，成功保留 owner 与屏蔽状态。
@details 自旋锁保持原先 pushOff/popOff、递归获取检查和 owner 协议。try_acquire 失败恢复中断嵌套，成功保留中断屏蔽至 release；acquire_spins 返回失败原子交换次数，不在无竞争路径读取时钟。原 acquire 调用者行为保留。
```

- [`xv::sync::SpinLock::acquire_spins`](spinlock.cpp.md#fn-ddedf097009cd32c)

- [`xv::sync::SpinLock::try_acquire`](spinlock.cpp.md#fn-1ed9206d4f9fc80e)
