# `kernel/process/resources/process_resources.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/resources/process_resources.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:resource_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::DescriptorTable::DuplicateFlow::found`](process_resources.cpp.md#fn-e99c0f2c82b36c7f)
- [`xv::process::DescriptorTable::DuplicateFlow::install`](process_resources.cpp.md#fn-e137d1c868e852b7)
- [`xv::process::DescriptorTable::DuplicateFlow::installed`](process_resources.cpp.md#fn-bc1b04d32f845775)
- [`xv::process::DescriptorTable::DuplicateFlow::lookup`](process_resources.cpp.md#fn-5171a91cfe9bcbfc)
- [`xv::process::DescriptorTable::DuplicateFlow::retain`](process_resources.cpp.md#fn-5a25e6c9d9d72ce3)
- [`xv::process::DescriptorTable::DuplicateFlow::rollback`](process_resources.cpp.md#fn-33a8add509dcdb12)
- [`xv::process::DescriptorTable::close`](process_resources.cpp.md#fn-315befdfc4ab2c6d)
- [`xv::process::DescriptorTable::close_all`](process_resources.cpp.md#fn-1a9397ea7f7cd7f3)
- [`xv::process::DescriptorTable::duplicate`](process_resources.cpp.md#fn-bf8e7f337214198f)
- [`xv::process::DescriptorTable::get`](process_resources.cpp.md#fn-5eee207e9f21abc9)
- [`xv::process::DescriptorTable::inherit_from`](process_resources.cpp.md#fn-067c825dc4aed5b6)
- [`xv::process::DescriptorTable::inspect`](process_resources.cpp.md#fn-7de3cc7234d49dc7)
- [`xv::process::DescriptorTable::install`](process_resources.cpp.md#fn-9ff2e78e57ca3144)
- [`xv::process::DescriptorTable::snapshot`](process_resources.cpp.md#fn-a133852fe02f0f86)
- [`xv::process::DescriptorTable::take`](process_resources.cpp.md#fn-5f35dd820658a6cc)
- [`xv::process::WorkingDirectory::clear`](process_resources.cpp.md#fn-3db3072ad2b9394a)
- [`xv::process::WorkingDirectory::duplicate`](process_resources.cpp.md#fn-4f397a6c89c47ea5)
- [`xv::process::WorkingDirectory::inherit_from`](process_resources.cpp.md#fn-1359f20c72f6c9b8)
- [`xv::process::WorkingDirectory::replace`](process_resources.cpp.md#fn-0c82862334d7f976)

<a id="fn-5eee207e9f21abc9"></a>

## `xv::process::DescriptorTable::get`

```doxygen
@fn xv::fs::File *xv::process::DescriptorTable::get(int descriptor) const
@brief 在描述符表锁内取得一个借用的 File 指针。
@details 只供所属进程的单一执行线程继续使用；锁释放后借用不保护任意跨进程访问，观测者应使用 inspect，继承应使用 snapshot。
```

<a id="fn-9ff2e78e57ca3144"></a>

## `xv::process::DescriptorTable::install`

```doxygen
@fn int xv::process::DescriptorTable::install(xv::fs::File *owned)
@brief 在描述符表锁内安装一个已拥有的文件引用。
@details 寻找空槽，成功接管引用并返回 fd，失败保持调用者拥有权。未完成绑定的 File 由发布标志阻止被观测。
```

<a id="fn-5f35dd820658a6cc"></a>

## `xv::process::DescriptorTable::take`

```doxygen
@fn xv::fs::File *xv::process::DescriptorTable::take(int descriptor)
@brief 在描述符表锁内撤下并转交一个引用。
@details 撤下后才在外层执行可能睡眠的 File::release/close，不能把后端关闭放进描述符自旋锁。
```

<a id="fn-315befdfc4ab2c6d"></a>

## `xv::process::DescriptorTable::close`

```doxygen
@fn bool xv::process::DescriptorTable::close(int descriptor)
@brief 进程文件描述符表：关闭当前端点或释放文件关联。
@param descriptor 本次操作使用的 descriptor 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`take` → `detail::release_file`。
@details 需要特别跟踪的条件：`!owned`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bf8e7f337214198f"></a>

## `xv::process::DescriptorTable::duplicate`

```doxygen
@fn int xv::process::DescriptorTable::duplicate(int descriptor)
@brief 进程文件描述符表：复制引用。
@param descriptor 本次操作使用的 descriptor 值。
@return 各分支实际返回：`-1`；`result`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DuplicateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-1a9397ea7f7cd7f3"></a>

## `xv::process::DescriptorTable::close_all`

```doxygen
@fn void xv::process::DescriptorTable::close_all()
@brief 进程文件描述符表：closeall。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`close`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-067c825dc4aed5b6"></a>

## `xv::process::DescriptorTable::inherit_from`

```doxygen
@fn void xv::process::DescriptorTable::inherit_from(const DescriptorTable &parent)
@brief 为未发布子进程复制父进程的描述符引用。
@details 拒绝自继承或非空目标，使用父表 snapshot 在一把表锁内增加所有文件引用，保留 dup/fork 的共享 File 语义。子进程的发布仍由进程锁与调度流程保护。
```

<a id="fn-4f397a6c89c47ea5"></a>

## `xv::process::WorkingDirectory::duplicate`

```doxygen
@fn xv::fs::Inode *xv::process::WorkingDirectory::duplicate() const
@brief 进程工作目录引用：读取或计算 `directory_ ? detail::retain_inode(directory_) : nullptr`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::retain_inode`。
@return 各分支实际返回：`directory_ ? detail::retain_inode(directory_) : nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0c82862334d7f976"></a>

## `xv::process::WorkingDirectory::replace`

```doxygen
@fn void xv::process::WorkingDirectory::replace(xv::fs::Inode *owned)
@brief 进程工作目录引用：替换。
@param owned 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::release_inode`。
@details 本函数可见的状态更新包括：`directory_ = owned;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`old`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3db3072ad2b9394a"></a>

## `xv::process::WorkingDirectory::clear`

```doxygen
@fn void xv::process::WorkingDirectory::clear()
@brief 进程工作目录引用：清空当前集合、指针或映射状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`replace`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1359f20c72f6c9b8"></a>

## `xv::process::WorkingDirectory::inherit_from`

```doxygen
@fn void xv::process::WorkingDirectory::inherit_from(const WorkingDirectory &parent)
@brief 进程工作目录引用：继承来源。
@param parent 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::resource_fail` → `parent.duplicate`。
@details 本函数可见的状态更新包括：`directory_ = parent.duplicate();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`directory_ || this == &parent`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
<a id="fn-a133852fe02f0f86"></a>

## `xv::process::DescriptorTable::snapshot`

```doxygen
@fn unsigned xv::process::DescriptorTable::snapshot(xv::fs::File **references, unsigned capacity) const
@brief 在表锁内复制并保留各 fd 的文件引用。
@details 只调用不会睡眠的 retain_file，输出按原 fd 下标排列，调用者负责在锁外归还引用。用于 fork 继承；procfs 元数据观测使用不转移引用的 inspect。
```

<a id="fn-7de3cc7234d49dc7"></a>

## `xv::process::DescriptorTable::inspect`

```doxygen
@fn unsigned xv::process::DescriptorTable::inspect(xv::fs::FileInfo *entries, unsigned capacity) const
@brief 在表锁内复制各 fd 的已发布身份和标志。
@details 描述符槽的现存引用在锁内保持 File 存活；元数据查询不加节点锁、不执行 stat/close。输出空项清零，返回有效项数；因此观测不会延长目标文件寿命或触发最后关闭。
```

<a id="fn-e99c0f2c82b36c7f"></a>

## `xv::process::DescriptorTable::DuplicateFlow::found`

```doxygen
@fn static bool xv::process::DescriptorTable::DuplicateFlow::found(Context &c) noexcept
@brief 检查描述符是否指向有效文件。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e137d1c868e852b7"></a>

## `xv::process::DescriptorTable::DuplicateFlow::install`

```doxygen
@fn static void xv::process::DescriptorTable::DuplicateFlow::install(Context &c) noexcept
@brief 尝试将候选引用安装到最低空闲槽。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bc1b04d32f845775"></a>

## `xv::process::DescriptorTable::DuplicateFlow::installed`

```doxygen
@fn static bool xv::process::DescriptorTable::DuplicateFlow::installed(Context &c) noexcept
@brief 检查新描述符是否安装成功。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5171a91cfe9bcbfc"></a>

## `xv::process::DescriptorTable::DuplicateFlow::lookup`

```doxygen
@fn static void xv::process::DescriptorTable::DuplicateFlow::lookup(Context &c) noexcept
@brief 取得指定描述符的借用文件指针。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5a25e6c9d9d72ce3"></a>

## `xv::process::DescriptorTable::DuplicateFlow::retain`

```doxygen
@fn static void xv::process::DescriptorTable::DuplicateFlow::retain(Context &c) noexcept
@brief 为候选新描述符保留一个文件引用。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-33a8add509dcdb12"></a>

## `xv::process::DescriptorTable::DuplicateFlow::rollback`

```doxygen
@fn static void xv::process::DescriptorTable::DuplicateFlow::rollback(Context &c) noexcept
@brief 安装失败时释放候选新引用。
@details 保留原 get、retain、install 次序；安装失败只释放新保留引用，原表项及调用者引用不变。 下一节点由图策略声明，业务结果与控制信号分离。
```
