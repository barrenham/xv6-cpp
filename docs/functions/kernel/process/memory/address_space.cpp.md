# `kernel/process/memory/address_space.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/memory/address_space.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::AddressSpace::copy_in`](address_space.cpp.md#fn-78195cebfcf9cc5a)
- [`xv::process::AddressSpace::copy_out`](address_space.cpp.md#fn-af80ee5a35af7f0e)
- [`xv::process::AddressSpace::copy_string`](address_space.cpp.md#fn-99aa1b54bd7ccfab)
- [`xv::process::AddressSpace::copy_to`](address_space.cpp.md#fn-f441ea3cfe631614)
- [`xv::process::AddressSpace::create`](address_space.cpp.md#fn-ac548aaaa82b1697)
- [`xv::process::AddressSpace::grow_to`](address_space.cpp.md#fn-6a2eebb0fa906335)
- [`xv::process::AddressSpace::handle_fault`](address_space.cpp.md#fn-ef1a8c8627cc3494)
- [`xv::process::AddressSpace::protect_guard`](address_space.cpp.md#fn-55c0283e7aac7f96)
- [`xv::process::AddressSpace::reset`](address_space.cpp.md#fn-71495b3b41ddb9ad)
- [`xv::process::AddressSpace::resize`](address_space.cpp.md#fn-75a8c7ea8deccbc3)
- [`xv::process::AddressSpace::swap`](address_space.cpp.md#fn-5fd205292faac723)
- [`xv::process::AddressSpace::translate`](address_space.cpp.md#fn-323d412cddaf0e22)

<a id="fn-ac548aaaa82b1697"></a>

## `xv::process::AddressSpace::create`

```doxygen
@fn bool xv::process::AddressSpace::create(void *trap_frame, void *user_info)
@brief 创建用户页表并登记特殊映射。
@details 沿用 trampoline、trapframe 和只读用户 PID 页映射，再登记各自的访问权限；元数据失败通过 reset 回收全部候选资源，不发布半成品地址空间。
@details 编排由 CreateFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-71495b3b41ddb9ad"></a>

## `xv::process::AddressSpace::reset`

```doxygen
@fn void xv::process::AddressSpace::reset()
@brief 归还用户映射、页表和地址区域登记。
@details 先移除外部拥有的特殊映射，再按用户帧共享引用释放其余映射与页表，最后清除 size、heap 起点和区域元数据。reset 显式结束所有权，调用者保证无运行中用户线程。
```

<a id="fn-5fd205292faac723"></a>

## `xv::process::AddressSpace::swap`

```doxygen
@fn void xv::process::AddressSpace::swap(AddressSpace &other)
@brief 原子提交协议下交换镜像及其区域归属。
@details 交换页表、size、heap 起点和 VirtualAddressPool 页链，再发布双方观测长度；调用方独占进程镜像，操作自身不分配内存。
```

<a id="fn-f441ea3cfe631614"></a>

## `xv::process::AddressSpace::copy_to`

```doxygen
@fn bool xv::process::AddressSpace::copy_to(AddressSpace &child) const
@brief 复制区域登记并以 COW 共享用户帧。
@details 先显式复制区域元数据，再建立共享用户页；成功后发布子 size 和 heap 起点。失败由候选进程回收其页表和元数据，父表残留 COW 允许以后独占提升。
@details 编排由 CopyFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-75a8c7ea8deccbc3"></a>

## `xv::process::AddressSpace::resize`

```doxygen
@fn int xv::process::AddressSpace::resize(int bytes, bool eager)
@brief 在已登记堆范围内调整逻辑 break。
@details 正增长先登记页级 RWU 匿名区域，再按 eager 决定是否分配数据帧；失败撤销新增范围。负增长拒绝下溢和越过初始镜像/栈起点，删除区域后释放映射。只有成功才发布新 size。
@details 编排由 ResizeFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-6a2eebb0fa906335"></a>

## `xv::process::AddressSpace::grow_to`

```doxygen
@fn bool xv::process::AddressSpace::grow_to(unsigned long end, int permissions, bool stack)
@brief 为候选镜像或用户栈登记并建立新映射。
@details 拒绝倒退和超过 UTOP，登记页对齐范围及真实权限，分配失败撤销登记。成功更新 size 和初始 heap 起点；stack 参数区分用户栈与镜像区域。
@details 编排由 GrowFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-55c0283e7aac7f96"></a>

## `xv::process::AddressSpace::protect_guard`

```doxygen
@fn bool xv::process::AddressSpace::protect_guard(unsigned long address)
@brief 同时撤销保护页的区域权限与用户访问位。
@details 先删除原登记并插入无权限 Guard，元数据失败返回 false，候选装载器据此放弃镜像；成功后清除 PTE_U，不把保护页变成可按需补页的空洞。
@details 编排由 GuardFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-323d412cddaf0e22"></a>

## `xv::process::AddressSpace::translate`

```doxygen
@fn unsigned long xv::process::AddressSpace::translate(unsigned long address) const
@brief 进程用户地址空间：解析当前页表或地址空间中的地址。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::translate_user_page`。
@return 各分支实际返回：`xv::memory::translate_user_page(table_, address)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ef1a8c8627cc3494"></a>

## `xv::process::AddressSpace::handle_fault`

```doxygen
@fn xv::memory::FaultOutcome xv::process::AddressSpace::handle_fault(unsigned long address, xv::memory::FaultAccess access)
@brief 把带明确访问类型的用户异常转入区域感知解析器。
@details 传递当前页表、逻辑大小和区域表，返回 FaultOutcome 而非把任何现存映射误当作处理成功；调用方只在 Resolved 时重试。
```

<a id="fn-78195cebfcf9cc5a"></a>

## `xv::process::AddressSpace::copy_in`

```doxygen
@fn int xv::process::AddressSpace::copy_in(void *destination, unsigned long source, unsigned long bytes) const
@brief 在本地址空间中按权限读取用户缓冲。
@details 把区域表与逻辑长度交给 copy_from_user，保持懒分配、区域边界和实际 PTE 权限一致。
```

<a id="fn-af80ee5a35af7f0e"></a>

## `xv::process::AddressSpace::copy_out`

```doxygen
@fn int xv::process::AddressSpace::copy_out(unsigned long destination, const void *source, unsigned long bytes) const
@brief 在本地址空间中写入用户缓冲并处理 COW。
@details 把区域表与逻辑长度交给 copy_to_user，写入前必要时私有化共享帧；不允许借内核复制绕过只读或保护页。
```

<a id="fn-99aa1b54bd7ccfab"></a>

## `xv::process::AddressSpace::copy_string`

```doxygen
@fn int xv::process::AddressSpace::copy_string(char *destination, unsigned long source, unsigned long limit) const
@brief 按本地址空间权限读取用户终止字符串。
@details 将区域登记和逻辑长度一起传入 copy_user_string，支持合法跨页懒分配，拒绝跨出登记范围。
```
