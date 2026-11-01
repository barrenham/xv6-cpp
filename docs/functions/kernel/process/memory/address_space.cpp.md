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
@details 沿用 trampoline、trapframe 和只读用户 PID 页映射，再登记各自的访问权限；元数据失败通过 reset 回收全部候选资源，不发布半成品地址空间。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@details 编排由 CreateFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-71495b3b41ddb9ad"></a>

## `xv::process::AddressSpace::reset`

```doxygen
@fn void xv::process::AddressSpace::reset()
@brief 归还用户映射、页表和地址区域登记。
@details 先移除外部拥有的特殊映射，再按用户帧共享引用释放其余映射与页表，最后清除 size、heap 起点和区域元数据。reset 显式结束所有权，调用者保证无运行中用户线程。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-5fd205292faac723"></a>

## `xv::process::AddressSpace::swap`

```doxygen
@fn void xv::process::AddressSpace::swap(AddressSpace &other)
@brief 原子提交协议下交换镜像及其区域归属。
@details 交换页表、size、heap 起点和 VirtualAddressPool 页链，再发布双方观测长度；调用方独占进程镜像，操作自身不分配内存。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-f441ea3cfe631614"></a>

## `xv::process::AddressSpace::copy_to`

```doxygen
@fn bool xv::process::AddressSpace::copy_to(AddressSpace &child) const
@brief 复制区域登记并以 COW 共享用户帧。
@details 先显式复制区域元数据，再建立共享用户页；成功后发布子 size 和 heap 起点。失败由候选进程回收其页表和元数据，父表残留 COW 允许以后独占提升。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@details 编排由 CopyFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-75a8c7ea8deccbc3"></a>

## `xv::process::AddressSpace::resize`

```doxygen
@fn int xv::process::AddressSpace::resize(int bytes, bool eager)
@brief 在已登记堆范围内调整逻辑 break。
@details 正增长先登记页级 RWU 匿名区域，再按 eager 决定是否分配数据帧；失败撤销新增范围。负增长拒绝下溢和越过初始镜像/栈起点，删除区域后释放映射。只有成功才发布新 size。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@details 编排由 ResizeFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-6a2eebb0fa906335"></a>

## `xv::process::AddressSpace::grow_to`

```doxygen
@fn bool xv::process::AddressSpace::grow_to(unsigned long end, int permissions, bool stack)
@brief 为候选镜像或用户栈登记并建立新映射。
@details 拒绝倒退和超过 UTOP，登记页对齐范围及真实权限，分配失败撤销登记。成功更新 size 和初始 heap 起点；stack 参数区分用户栈与镜像区域。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@details 编排由 GrowFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-55c0283e7aac7f96"></a>

## `xv::process::AddressSpace::protect_guard`

```doxygen
@fn bool xv::process::AddressSpace::protect_guard(unsigned long address)
@brief 同时撤销保护页的区域权限与用户访问位。
@details 先删除原登记并插入无权限 Guard，元数据失败返回 false，候选装载器据此放弃镜像；成功后清除 PTE_U，不把保护页变成可按需补页的空洞。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@details 编排由 GuardFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-323d412cddaf0e22"></a>

## `xv::process::AddressSpace::translate`

```doxygen
@fn unsigned long xv::process::AddressSpace::translate(unsigned long address) const
@brief 进程用户地址空间：解析当前页表或地址空间中的地址。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::translate_user_page`。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
@return 各分支实际返回：`xv::memory::translate_user_page(table_, address)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ef1a8c8627cc3494"></a>

## `xv::process::AddressSpace::handle_fault`

```doxygen
@fn xv::memory::FaultOutcome xv::process::AddressSpace::handle_fault(unsigned long address, xv::memory::FaultAccess access)
@brief 把带明确访问类型的用户异常转入区域感知解析器。
@details 传递当前页表、逻辑大小和区域表，返回 FaultOutcome 而非把任何现存映射误当作处理成功；调用方只在 Resolved 时重试。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-78195cebfcf9cc5a"></a>

## `xv::process::AddressSpace::copy_in`

```doxygen
@fn int xv::process::AddressSpace::copy_in(void *destination, unsigned long source, unsigned long bytes) const
@brief 在本地址空间中按权限读取用户缓冲。
@details 把区域表与逻辑长度交给 copy_from_user，保持懒分配、区域边界和实际 PTE 权限一致。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-af80ee5a35af7f0e"></a>

## `xv::process::AddressSpace::copy_out`

```doxygen
@fn int xv::process::AddressSpace::copy_out(unsigned long destination, const void *source, unsigned long bytes) const
@brief 在本地址空间中写入用户缓冲并处理 COW。
@details 把区域表与逻辑长度交给 copy_to_user，写入前必要时私有化共享帧；不允许借内核复制绕过只读或保护页。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-99aa1b54bd7ccfab"></a>

## `xv::process::AddressSpace::copy_string`

```doxygen
@fn int xv::process::AddressSpace::copy_string(char *destination, unsigned long source, unsigned long limit) const
@brief 按本地址空间权限读取用户终止字符串。
@details 将区域登记和逻辑长度一起传入 copy_user_string，支持合法跨页懒分配，拒绝跨出登记范围。 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-fa4426fa4e5caabb"></a>

## `xv::process::AddressSpace::map_device_pages`

```doxygen
@fn unsigned long xv::process::AddressSpace::map_device_pages(void *const *pages, unsigned count)
@brief 创建可读写、不可执行的共享设备映射；重复同一映射返回既有地址。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-354924ecaf09fe35"></a>

## `xv::process::AddressSpace::device_mapping`

```doxygen
@fn bool xv::process::AddressSpace::device_mapping(void *const *pages, unsigned count) const
@brief 比较固定窗口所持帧集合的首帧身份和页数；设备帧集合建立后保持不变。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-fe25ab9d20f5ce0a"></a>

## `xv::process::AddressSpace::unmap_device_pages`

```doxygen
@fn bool xv::process::AddressSpace::unmap_device_pages()
@brief 移除设备窗口叶映射、释放每页引用并归还区域；不存在时返回false。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-e6e7125a31b36c94"></a>

## `xv::process::AddressSpace::DeviceMapFlow::reserve`

```doxygen
@fn static bool xv::process::AddressSpace::DeviceMapFlow::reserve(Context &c) noexcept
@brief 验证表、页数及无旧映射，并预留设备区域；继承路径沿用已克隆区域。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-1f227b88ce994e19"></a>

## `xv::process::AddressSpace::DeviceMapFlow::map`

```doxygen
@fn static bool xv::process::AddressSpace::DeviceMapFlow::map(Context &c) noexcept
@brief 以用户可读写权限映射当前散页，不设置执行或COW。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-b2cf60a0b2fcb6d8"></a>

## `xv::process::AddressSpace::DeviceMapFlow::retain`

```doxygen
@fn static void xv::process::AddressSpace::DeviceMapFlow::retain(Context &c) noexcept
@brief 成功映射后保留帧引用，记录首帧身份和已完成前缀。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-2c6a9133c45fe3fe"></a>

## `xv::process::AddressSpace::DeviceMapFlow::finished`

```doxygen
@fn static bool xv::process::AddressSpace::DeviceMapFlow::finished(Context &c) noexcept
@brief 判断全部设备页是否映射完成。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-e65bfbe430939f29"></a>

## `xv::process::AddressSpace::DeviceMapFlow::commit`

```doxygen
@fn static void xv::process::AddressSpace::DeviceMapFlow::commit(Context &c) noexcept
@brief 发布设备窗口起始地址。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-b958d00b21e0b685"></a>

## `xv::process::AddressSpace::DeviceMapFlow::rollback`

```doxygen
@fn static void xv::process::AddressSpace::DeviceMapFlow::rollback(Context &c) noexcept
@brief 撤销已经成功映射的前缀并释放区域；没有前缀时只撤销预留。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

<a id="fn-03c9db31cea2af38"></a>

## `xv::process::AddressSpace::map_device`

```doxygen
@fn unsigned long xv::process::AddressSpace::map_device(void *const *pages, unsigned count, const AddressSpace *inherited)
@brief 运行DeviceMapFlow完成区域预留、逐页映射及引用保留，失败回滚。
@details 设备窗口位于UTOP下方32MiB处，最多4096页，使用RWU Special区域以独立于堆大小参与用户访问检查；没有执行权限。页引用由设备与每个地址空间分别持有，fork显式继承共享可写页而不转为COW；swap交接计数/身份，exec/reset/退出或显式unmap释放引用。页表失败只撤销已成功保留的前缀，空页表目录仍由地址空间最终释放。
```

- [`xv::process::AddressSpace::map_device_pages`](address_space.cpp.md#fn-fa4426fa4e5caabb)

- [`xv::process::AddressSpace::device_mapping`](address_space.cpp.md#fn-354924ecaf09fe35)

- [`xv::process::AddressSpace::unmap_device_pages`](address_space.cpp.md#fn-fe25ab9d20f5ce0a)

- [`xv::process::AddressSpace::DeviceMapFlow::reserve`](address_space.cpp.md#fn-e6e7125a31b36c94)

- [`xv::process::AddressSpace::DeviceMapFlow::map`](address_space.cpp.md#fn-1f227b88ce994e19)

- [`xv::process::AddressSpace::DeviceMapFlow::retain`](address_space.cpp.md#fn-b2cf60a0b2fcb6d8)

- [`xv::process::AddressSpace::DeviceMapFlow::finished`](address_space.cpp.md#fn-2c6a9133c45fe3fe)

- [`xv::process::AddressSpace::DeviceMapFlow::commit`](address_space.cpp.md#fn-e65bfbe430939f29)

- [`xv::process::AddressSpace::DeviceMapFlow::rollback`](address_space.cpp.md#fn-b958d00b21e0b685)

- [`xv::process::AddressSpace::map_device`](address_space.cpp.md#fn-03c9db31cea2af38)
