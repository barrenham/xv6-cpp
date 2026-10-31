# `kernel/process/storage/paged_process_store.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/storage/paged_process_store.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:paged_store`, `:storage_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::PagedProcessStore::AllocateFlow::construct`](paged_process_store.cpp.md#fn-159ab3119d8156b5)
- [`xv::process::PagedProcessStore::AllocateFlow::has_page`](paged_process_store.cpp.md#fn-8cc0bf2c16fb13e4)
- [`xv::process::PagedProcessStore::AllocateFlow::has_stack`](paged_process_store.cpp.md#fn-1199815cd7310cfc)
- [`xv::process::PagedProcessStore::AllocateFlow::page`](paged_process_store.cpp.md#fn-fc3ab79e1826eac5)
- [`xv::process::PagedProcessStore::AllocateFlow::publish`](paged_process_store.cpp.md#fn-8e66bc154e7394e8)
- [`xv::process::PagedProcessStore::AllocateFlow::rollback`](paged_process_store.cpp.md#fn-3728d96b78ab4760)
- [`xv::process::PagedProcessStore::AllocateFlow::stack`](paged_process_store.cpp.md#fn-8250f0d0f78d17f9)
- [`xv::process::PagedProcessStore::ReleaseFlow::destroy`](paged_process_store.cpp.md#fn-7d9cabc95ddcd89f)
- [`xv::process::PagedProcessStore::ReleaseFlow::free_page`](paged_process_store.cpp.md#fn-284a139f0282cccf)
- [`xv::process::PagedProcessStore::ReleaseFlow::free_stack`](paged_process_store.cpp.md#fn-d471fc848b0f4fa7)
- [`xv::process::PagedProcessStore::ReleaseFlow::remove`](paged_process_store.cpp.md#fn-8367ea42f24740c2)
- [`xv::process::PagedProcessStore::ReleaseFlow::retire`](paged_process_store.cpp.md#fn-3c4deec9c966df92)
- [`xv::process::PagedProcessStore::allocate`](paged_process_store.cpp.md#fn-8add4ef4af29f162)
- [`xv::process::PagedProcessStore::count`](paged_process_store.cpp.md#fn-06351df5779372e3)
- [`xv::process::PagedProcessStore::find`](paged_process_store.cpp.md#fn-bffa9ef733537f45)
- [`xv::process::PagedProcessStore::initialize`](paged_process_store.cpp.md#fn-06625233c22e977d)
- [`xv::process::PagedProcessStore::node_at`](paged_process_store.cpp.md#fn-78087e7747e4c8ef)
- [`xv::process::PagedProcessStore::process_at`](paged_process_store.cpp.md#fn-9fad93a93800939d)
- [`xv::process::PagedProcessStore::release`](paged_process_store.cpp.md#fn-b890f202b0802620)
- [`xv::process::PagedProcessStore::visit`](paged_process_store.cpp.md#fn-6cb9aa1e15885253)

- [`xv::process::PagedProcessStore::AllocateFlow::published`](paged_process_store.cpp.md#fn-3fcf4fdd6acd7cdf)

- [`xv::process::PagedProcessStore::AllocateFlow::discard`](paged_process_store.cpp.md#fn-e05c51cba3e813ed)

- [`xv::process::PagedProcessStore::find_pid`](paged_process_store.cpp.md#fn-6dab1bac2166ed48)

<a id="fn-06625233c22e977d"></a>

## `xv::process::PagedProcessStore::initialize`

```doxygen
@fn void xv::process::PagedProcessStore::initialize()
@brief 在发布前验证对象大小、对齐及页内容量，绑定物理分配器和空页上限，不提前申请 slab 页。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-9fad93a93800939d"></a>

## `xv::process::PagedProcessStore::process_at`

```doxygen
@fn Process *xv::process::PagedProcessStore::process_at(Node &node)
@brief 稳定的分页进程登记：读取或计算 `__builtin_launder(reinterpret_cast<Process *>(node.storage))`，返回当前查询结果。
@param node inode 或链表节点对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__builtin_launder`。
@return 各分支实际返回：`__builtin_launder(reinterpret_cast<Process *>(node.storage))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-78087e7747e4c8ef"></a>

## `xv::process::PagedProcessStore::node_at`

```doxygen
@fn PagedProcessStore::Node *xv::process::PagedProcessStore::node_at(Process &process)
@brief 通过进程中不变的存储归属指针找回节点，不依赖非标准布局的成员偏移。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-8add4ef4af29f162"></a>

## `xv::process::PagedProcessStore::allocate`

```doxygen
@fn Process *xv::process::PagedProcessStore::allocate(int pid)
@brief 通过 AllocateFlow 取得 slab 描述符和按需内核栈，在登记锁内共同发布 PID 树与双向登记链。负 PID 拒绝；重复正 PID 完整回滚；零表示内部未索引测试对象。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-b890f202b0802620"></a>

## `xv::process::PagedProcessStore::release`

```doxygen
@fn void xv::process::PagedProcessStore::release(Process *process)
@brief 先核对不可变节点归属，拒绝跨注册表释放；随后消费持有的进程锁，通过 ReleaseFlow 退役、摘除登记链与 PID 树并完整回收。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-bffa9ef733537f45"></a>

## `xv::process::PagedProcessStore::find`

```doxygen
@fn Process *xv::process::PagedProcessStore::find(ProcessPredicate &predicate)
@brief 稳定的分页进程登记：查找满足选择器的稳定对象。
@param predicate 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `process_at` → `lock` → `predicate.matches` → `unlock`。
@details 需要特别跟踪的条件：`predicate.matches(*process)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`process`；`nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6cb9aa1e15885253"></a>

## `xv::process::PagedProcessStore::visit`

```doxygen
@fn void xv::process::PagedProcessStore::visit(ProcessVisitor &visitor)
@brief 稳定的分页进程登记：处理稳定遍历当前访问到的对象。
@param visitor 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `process_at` → `lock` → `visitor.visit` → `unlock`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-06351df5779372e3"></a>

## `xv::process::PagedProcessStore::count`

```doxygen
@fn unsigned long xv::process::PagedProcessStore::count() const
@brief 稳定的分页进程登记：读取或计算 `count_`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@return 各分支实际返回：`count_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-159ab3119d8156b5"></a>

## `xv::process::PagedProcessStore::AllocateFlow::construct`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::construct(Context &c) noexcept
@brief 构造尚未公开的节点和描述符，在发布前写入 PID、存储归属与内核栈地址。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-8cc0bf2c16fb13e4"></a>

## `xv::process::PagedProcessStore::AllocateFlow::has_page`

```doxygen
@fn static bool xv::process::PagedProcessStore::AllocateFlow::has_page(Context &c) noexcept
@brief 检查候选页分配是否成功。
@details 按原顺序取得描述符页与内核栈，栈失败只归还候选页；构造并持锁初始化后才在注册表锁内发布。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1199815cd7310cfc"></a>

## `xv::process::PagedProcessStore::AllocateFlow::has_stack`

```doxygen
@fn static bool xv::process::PagedProcessStore::AllocateFlow::has_stack(Context &c) noexcept
@brief 检查内核栈分配是否成功。
@details 按原顺序取得描述符页与内核栈，栈失败只归还候选页；构造并持锁初始化后才在注册表锁内发布。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fc3ab79e1826eac5"></a>

## `xv::process::PagedProcessStore::AllocateFlow::page`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::page(Context &c) noexcept
@brief 取得描述符候选页。
@details 按原顺序取得描述符页与内核栈，栈失败只归还候选页；构造并持锁初始化后才在注册表锁内发布。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8e66bc154e7394e8"></a>

## `xv::process::PagedProcessStore::AllocateFlow::publish`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::publish(Context &c) noexcept
@brief 候选尚未公开且持有自身进程锁，在登记锁内插入唯一 PID 树和双向链表；重复 PID 不改变登记结构并交给 discard 回滚。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-3728d96b78ab4760"></a>

## `xv::process::PagedProcessStore::AllocateFlow::rollback`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::rollback(Context &c) noexcept
@brief 栈分配失败时归还尚未发布的候选页。
@details 按原顺序取得描述符页与内核栈，栈失败只归还候选页；构造并持锁初始化后才在注册表锁内发布。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8250f0d0f78d17f9"></a>

## `xv::process::PagedProcessStore::AllocateFlow::stack`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::stack(Context &c) noexcept
@brief 取得内核栈地址。
@details 按原顺序取得描述符页与内核栈，栈失败只归还候选页；构造并持锁初始化后才在注册表锁内发布。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7d9cabc95ddcd89f"></a>

## `xv::process::PagedProcessStore::ReleaseFlow::destroy`

```doxygen
@fn static void xv::process::PagedProcessStore::ReleaseFlow::destroy(Context &c) noexcept
@brief 释放排空锁、销毁进程和节点对象。
@details 保存节点与栈后先 retire 释放进程锁，再取注册表锁摘链并等待读者释放进程锁；销毁后不读取进程，最后释放栈和节点页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-284a139f0282cccf"></a>

## `xv::process::PagedProcessStore::ReleaseFlow::free_page`

```doxygen
@fn static void xv::process::PagedProcessStore::ReleaseFlow::free_page(Context &c) noexcept
@brief 归还节点页且不再访问页内对象。
@details 保存节点与栈后先 retire 释放进程锁，再取注册表锁摘链并等待读者释放进程锁；销毁后不读取进程，最后释放栈和节点页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d471fc848b0f4fa7"></a>

## `xv::process::PagedProcessStore::ReleaseFlow::free_stack`

```doxygen
@fn static void xv::process::PagedProcessStore::ReleaseFlow::free_stack(Context &c) noexcept
@brief 按预先保存地址归还内核栈。
@details 保存节点与栈后先 retire 释放进程锁，再取注册表锁摘链并等待读者释放进程锁；销毁后不读取进程，最后释放栈和节点页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8367ea42f24740c2"></a>

## `xv::process::PagedProcessStore::ReleaseFlow::remove`

```doxygen
@fn static void xv::process::PagedProcessStore::ReleaseFlow::remove(Context &c) noexcept
@brief 进程先退役并释放原进程锁；随后在登记锁内 O(1) 摘除双向登记链、O(log N) 删除 PID 索引，再等待此前借出的进程锁，阻止析构与查询竞争。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-3c4deec9c966df92"></a>

## `xv::process::PagedProcessStore::ReleaseFlow::retire`

```doxygen
@fn static void xv::process::PagedProcessStore::ReleaseFlow::retire(Context &c) noexcept
@brief 验证进程已脱离运行资源并标记退役、释放进程锁。
@details 保存节点与栈后先 retire 释放进程锁，再取注册表锁摘链并等待读者释放进程锁；销毁后不读取进程，最后释放栈和节点页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3fcf4fdd6acd7cdf"></a>

## `xv::process::PagedProcessStore::AllocateFlow::published`

```doxygen
@fn static bool xv::process::PagedProcessStore::AllocateFlow::published(Context &c) noexcept
@brief 判断 PID 索引和登记链是否已共同发布，重复 PID 时进入完整回滚。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-e05c51cba3e813ed"></a>

## `xv::process::PagedProcessStore::AllocateFlow::discard`

```doxygen
@fn static void xv::process::PagedProcessStore::AllocateFlow::discard(Context &c) noexcept
@brief 回滚未发布的重复 PID 候选：退役并析构描述符，归还内核栈与 slab 存储，结果清空；注册表和原 PID 不受影响。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-6dab1bac2166ed48"></a>

## `xv::process::PagedProcessStore::find_pid`

```doxygen
@fn Process *xv::process::PagedProcessStore::find_pid(int pid)
@brief 在登记锁内查询 PID 红黑树，再取得目标进程锁并复核身份；成功返回已锁定的稳定对象，失败返回空。删除沿用退役后摘除、等待借用锁归还的生命周期协议，查询为 O(log N)。
@note 详细锁协议见 docs/process-index-wait.md。
```
