# `kernel/process/storage/process_store.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/storage/process_store.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:store`, `:storage_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessStore::construct`](process_store.cpp.md#fn-8a84fd244ecbdafa)
- [`xv::process::ProcessStore::destroy`](process_store.cpp.md#fn-2b83f7e54a6d1383)
- [`xv::process::ProcessStore::lock`](process_store.cpp.md#fn-b1cb22bc8b45b706)
- [`xv::process::ProcessStore::registered`](process_store.cpp.md#fn-24f4b1215767875b)
- [`xv::process::ProcessStore::retire`](process_store.cpp.md#fn-002d09c9793e8249)
- [`xv::process::ProcessStore::set_stack`](process_store.cpp.md#fn-2f63d3d9cb1d8f33)
- [`xv::process::ProcessStore::unlock`](process_store.cpp.md#fn-ac479e8051ca7abb)

- [`xv::process::ProcessStore::storage_node`](process_store.cpp.md#fn-523816f8354db82c)

- [`xv::process::ProcessStore::has_pid`](process_store.cpp.md#fn-8bc24c83f3b7435b)

<a id="fn-8a84fd244ecbdafa"></a>

## `xv::process::ProcessStore::construct`

```doxygen
@fn Process *xv::process::ProcessStore::construct(void *storage, int pid, void *owner)
@brief 在调用者提供的存储中构造并锁住 Used 描述符，写入 PID 和存储归属；身份在注册表发布前确定。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-2b83f7e54a6d1383"></a>

## `xv::process::ProcessStore::destroy`

```doxygen
@fn void xv::process::ProcessStore::destroy(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：结束对象生命周期而不执行通用堆释放。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Process`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b1cb22bc8b45b706"></a>

## `xv::process::ProcessStore::lock`

```doxygen
@fn void xv::process::ProcessStore::lock(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：取得对象状态所需的互斥保护。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.lock.acquire`。
@note 源码中出现的锁操作：`process.lock.acquire`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ac479e8051ca7abb"></a>

## `xv::process::ProcessStore::unlock`

```doxygen
@fn void xv::process::ProcessStore::unlock(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：释放已持有的对象互斥保护。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.lock.release`。
@note 源码中出现的锁操作：`process.lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-002d09c9793e8249"></a>

## `xv::process::ProcessStore::retire`

```doxygen
@fn void xv::process::ProcessStore::retire(Process &process)
@brief 拒绝仍拥有亲属、事件、延迟通知、等待登记、组成员或调度所有权的描述符；要求持进程锁，标记 Unused 并交出锁，随后存储摘除索引并等待借用者完成。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-24f4b1215767875b"></a>

## `xv::process::ProcessStore::registered`

```doxygen
@fn bool xv::process::ProcessStore::registered(const Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：检查对象是否仍位于有效登记状态。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&process.wait_key_, __ATOMIC_ACQUIRE) != nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2f63d3d9cb1d8f33"></a>

## `xv::process::ProcessStore::set_stack`

```doxygen
@fn void xv::process::ProcessStore::set_stack(Process &process, unsigned long address)
@brief 进程对象生命周期、组身份、调度、等待与信号：设置栈。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 本函数可见的状态更新包括：`process.kstack = address;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-523816f8354db82c"></a>

## `xv::process::ProcessStore::storage_node`

```doxygen
@fn void *xv::process::ProcessStore::storage_node(const Process &process)
@brief 取得构造时写入且发布后不变的存储归属指针，避免对继承红黑树链接的非标准布局节点使用 offsetof。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-8bc24c83f3b7435b"></a>

## `xv::process::ProcessStore::has_pid`

```doxygen
@fn bool xv::process::ProcessStore::has_pid(const Process &process, int pid)
@brief 调用方已持进程锁时，核对描述符尚未退役且 PID 与查询键相同。
@note 详细锁协议见 docs/process-index-wait.md。
```
