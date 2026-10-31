# `kernel/syscall/sysfile.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysfile.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`xv.runtime`, `:arguments`, `:handlers`, `xv.fs`, `xv.process`, `xv.sync`, `xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::AccessFlow::assess`](sysfile.cpp.md#fn-5038a5cd17cfc6b3)
- [`xv::syscall::AccessFlow::lock`](sysfile.cpp.md#fn-2813b911c455e413)
- [`xv::syscall::AccessFlow::lookup`](sysfile.cpp.md#fn-ce479401f632b549)
- [`xv::syscall::AccessFlow::release`](sysfile.cpp.md#fn-48a7b755baa05774)
- [`xv::syscall::ArgumentVector::CopyFlow::advance`](sysfile.cpp.md#fn-91233eb73b6dfcbc)
- [`xv::syscall::ArgumentVector::CopyFlow::allocate`](sysfile.cpp.md#fn-c54cda5406bd960d)
- [`xv::syscall::ArgumentVector::CopyFlow::capacity`](sysfile.cpp.md#fn-9034897e6826ce87)
- [`xv::syscall::ArgumentVector::CopyFlow::copy_string`](sysfile.cpp.md#fn-87463b48c9777268)
- [`xv::syscall::ArgumentVector::CopyFlow::fetch`](sysfile.cpp.md#fn-40e32de0e45ea40f)
- [`xv::syscall::ArgumentVector::CopyFlow::present`](sysfile.cpp.md#fn-0f24651c03aaed27)
- [`xv::syscall::ArgumentVector::CopyFlow::success`](sysfile.cpp.md#fn-efe0ec5a57c4afae)
- [`xv::syscall::ArgumentVector::CopyFlow::terminated`](sysfile.cpp.md#fn-84defab755400773)
- [`xv::syscall::ArgumentVector::copy`](sysfile.cpp.md#fn-29f8037673a75725)
- [`xv::syscall::ArgumentVector::~ArgumentVector`](sysfile.cpp.md#fn-9d6ef88240dcea18)
- [`xv::syscall::AttributesFlow::assess`](sysfile.cpp.md#fn-591fcbfd108388bb)
- [`xv::syscall::AttributesFlow::commit`](sysfile.cpp.md#fn-211a3d44cf0d881d)
- [`xv::syscall::AttributesFlow::lock`](sysfile.cpp.md#fn-77562edf156a550b)
- [`xv::syscall::AttributesFlow::lookup`](sysfile.cpp.md#fn-40b30b2f6625ab95)
- [`xv::syscall::AttributesFlow::release`](sysfile.cpp.md#fn-b63955f962ea33a9)
- [`xv::syscall::AttributesFlow::valid`](sysfile.cpp.md#fn-7e454cabe9ec54aa)
- [`xv::syscall::CreateNodeFlow::create`](sysfile.cpp.md#fn-d4cab267092dc934)
- [`xv::syscall::CreateNodeFlow::release`](sysfile.cpp.md#fn-a4fa459b6c0396ea)
- [`xv::syscall::DirectoryFlow::commit`](sysfile.cpp.md#fn-8f0308f63d5de38e)
- [`xv::syscall::DirectoryFlow::lock`](sysfile.cpp.md#fn-58bfe15137cb3198)
- [`xv::syscall::DirectoryFlow::lookup`](sysfile.cpp.md#fn-aa0237f06b2f6349)
- [`xv::syscall::DirectoryFlow::release`](sysfile.cpp.md#fn-6c070fca366f5106)
- [`xv::syscall::DirectoryFlow::searchable`](sysfile.cpp.md#fn-64a45e417bcb835c)
- [`xv::syscall::ExecuteImageFlow::arguments`](sysfile.cpp.md#fn-65dcd7cb8075e930)
- [`xv::syscall::ExecuteImageFlow::environment`](sysfile.cpp.md#fn-20f444f4f32f8f8b)
- [`xv::syscall::ExecuteImageFlow::execute`](sysfile.cpp.md#fn-420cbc4a3992ebd4)
- [`xv::syscall::MountFlow::disk`](sysfile.cpp.md#fn-9f1803f1faaeda14)
- [`xv::syscall::MountFlow::mount_disk`](sysfile.cpp.md#fn-c5f23a8f6399d80f)
- [`xv::syscall::MountFlow::mount_ram`](sysfile.cpp.md#fn-cb43cf73881a02ed)
- [`xv::syscall::MountFlow::ram`](sysfile.cpp.md#fn-1c62b766d8985c01)
- [`xv::syscall::MountInfoFlow::bad_copy`](sysfile.cpp.md#fn-b43515f5dc5f0274)
- [`xv::syscall::MountInfoFlow::copy`](sysfile.cpp.md#fn-632ec74439b8aadd)
- [`xv::syscall::MountInfoFlow::describe`](sysfile.cpp.md#fn-df7270b008da891b)
- [`xv::syscall::MountInfoFlow::has_result`](sysfile.cpp.md#fn-0cff24bf64a3a2a6)
- [`xv::syscall::OpenFlow::allocate`](sysfile.cpp.md#fn-f14af60a05e2c6c1)
- [`xv::syscall::OpenFlow::bind`](sysfile.cpp.md#fn-b2dc915558ccaa53)
- [`xv::syscall::OpenFlow::create`](sysfile.cpp.md#fn-b1f61c339c709d16)
- [`xv::syscall::OpenFlow::creating`](sysfile.cpp.md#fn-80f11e622ca85ecd)
- [`xv::syscall::OpenFlow::detach`](sysfile.cpp.md#fn-8b1a00b5e3f01a24)
- [`xv::syscall::OpenFlow::existing_kind`](sysfile.cpp.md#fn-9c7aa5cc011e6f2d)
- [`xv::syscall::OpenFlow::install`](sysfile.cpp.md#fn-fc4d2763f37b5724)
- [`xv::syscall::OpenFlow::lock`](sysfile.cpp.md#fn-577aeb88257b0148)
- [`xv::syscall::OpenFlow::lookup`](sysfile.cpp.md#fn-9069da102898b0d4)
- [`xv::syscall::OpenFlow::permitted`](sysfile.cpp.md#fn-37aa15003f34302b)
- [`xv::syscall::OpenFlow::release_file`](sysfile.cpp.md#fn-1a1b50a8b73bd583)
- [`xv::syscall::OpenFlow::release_inode`](sysfile.cpp.md#fn-8cdde0c43d301c71)
- [`xv::syscall::OpenFlow::success`](sysfile.cpp.md#fn-d3b059ad234e3c53)
- [`xv::syscall::OpenFlow::truncate`](sysfile.cpp.md#fn-77ff83112dd9e92f)
- [`xv::syscall::OpenFlow::truncating`](sysfile.cpp.md#fn-9635ae189c28bdc2)
- [`xv::syscall::OpenFlow::writable_mount`](sysfile.cpp.md#fn-e32b35482a89e650)
- [`xv::syscall::PipeFlow::copy_reader`](sysfile.cpp.md#fn-292b04e3efac928d)
- [`xv::syscall::PipeFlow::copy_writer`](sysfile.cpp.md#fn-0eeea4b20ac2ee3c)
- [`xv::syscall::PipeFlow::create`](sysfile.cpp.md#fn-51a0e3c4224e7cca)
- [`xv::syscall::PipeFlow::detach_both`](sysfile.cpp.md#fn-412ae80dafe4e5f7)
- [`xv::syscall::PipeFlow::detach_reader`](sysfile.cpp.md#fn-8f80725730058215)
- [`xv::syscall::PipeFlow::install_reader`](sysfile.cpp.md#fn-c025ee0e05f02ccc)
- [`xv::syscall::PipeFlow::install_writer`](sysfile.cpp.md#fn-d42be175f81cf4d4)
- [`xv::syscall::PipeFlow::release`](sysfile.cpp.md#fn-58787419a89b2274)
- [`xv::syscall::PipeFlow::success`](sysfile.cpp.md#fn-01f8c6d4a33e5367)
- [`xv::syscall::PtyOpenFlow::allocate`](sysfile.cpp.md#fn-38541a888bfc07d7)
- [`xv::syscall::PtyOpenFlow::bind`](sysfile.cpp.md#fn-a10b56192b5db31d)
- [`xv::syscall::PtyOpenFlow::copy`](sysfile.cpp.md#fn-2c6e3eb6c886ed7b)
- [`xv::syscall::PtyOpenFlow::create`](sysfile.cpp.md#fn-2db65bf386d1a4bb)
- [`xv::syscall::PtyOpenFlow::detach`](sysfile.cpp.md#fn-0f9c8cbbedb9829f)
- [`xv::syscall::PtyOpenFlow::install`](sysfile.cpp.md#fn-c3cdacf7e4457981)
- [`xv::syscall::PtyOpenFlow::release`](sysfile.cpp.md#fn-3045ff1afe73e4ba)
- [`xv::syscall::PtyOpenFlow::success`](sysfile.cpp.md#fn-e922e669f63d689c)
- [`xv::syscall::ReadlinkFlow::lock`](sysfile.cpp.md#fn-deff4cbb5131c9fd)
- [`xv::syscall::ReadlinkFlow::lookup`](sysfile.cpp.md#fn-c9563bce4f63f3d9)
- [`xv::syscall::ReadlinkFlow::read`](sysfile.cpp.md#fn-e68cd13ce42653b3)
- [`xv::syscall::ReadlinkFlow::release`](sysfile.cpp.md#fn-a8f4d5b237f66fbe)
- [`xv::syscall::ReadlinkFlow::symlink`](sysfile.cpp.md#fn-6944b1a6106c266e)
- [`xv::syscall::StatFlow::copy`](sysfile.cpp.md#fn-579bc08c05755fd2)
- [`xv::syscall::StatFlow::lock`](sysfile.cpp.md#fn-c14adbd10597827e)
- [`xv::syscall::StatFlow::lookup`](sysfile.cpp.md#fn-1ded90600992a4e9)
- [`xv::syscall::StatFlow::release`](sysfile.cpp.md#fn-b4bd00ba23d091c6)
- [`xv::syscall::StatFlow::snapshot`](sysfile.cpp.md#fn-6593b020e780880e)
- [`xv::syscall::UnmountFlow::removed`](sysfile.cpp.md#fn-8d5cf746df533b95)
- [`xv::syscall::UnmountFlow::sync`](sysfile.cpp.md#fn-ccfb906896eb2fdb)
- [`xv::syscall::UnmountFlow::unmount`](sysfile.cpp.md#fn-56c8233a8d10a6f1)
- [`xv::syscall::argfd`](sysfile.cpp.md#fn-7564cff018f6113b)
- [`xv::syscall::attributes`](sysfile.cpp.md#fn-2093ad83f06783f7)
- [`xv::syscall::execute_image`](sysfile.cpp.md#fn-17d77b950c1f0b08)
- [`xv::syscall::fdalloc`](sysfile.cpp.md#fn-e1520dac81970d65)
- [`xv::syscall::sys_access`](sysfile.cpp.md#fn-58aee6d34f9fa96f)
- [`xv::syscall::sys_chdir`](sysfile.cpp.md#fn-5db6c22d9b99841b)
- [`xv::syscall::sys_chmod`](sysfile.cpp.md#fn-94978fea0ce1a092)
- [`xv::syscall::sys_chown`](sysfile.cpp.md#fn-61a96d5c2d403c15)
- [`xv::syscall::sys_close`](sysfile.cpp.md#fn-4692a7b4aafb1a42)
- [`xv::syscall::sys_dup`](sysfile.cpp.md#fn-d76ff7559f643ffa)
- [`xv::syscall::sys_exec`](sysfile.cpp.md#fn-d063aa5f0fd12e38)
- [`xv::syscall::sys_execve`](sysfile.cpp.md#fn-2037b9e8e05eb105)
- [`xv::syscall::sys_fcntl`](sysfile.cpp.md#fn-2940e0efab81491b)
- [`xv::syscall::sys_fstat`](sysfile.cpp.md#fn-c2ca80b2260b826c)
- [`xv::syscall::sys_fsync`](sysfile.cpp.md#fn-7cce73f1ff266383)
- [`xv::syscall::sys_ftruncate`](sysfile.cpp.md#fn-ac0849d188c61352)
- [`xv::syscall::sys_ioctl`](sysfile.cpp.md#fn-868f8327fa5e0719)
- [`xv::syscall::sys_link`](sysfile.cpp.md#fn-f0e4c3a2b61b65f2)
- [`xv::syscall::sys_lseek`](sysfile.cpp.md#fn-09832f54fc77b0ca)
- [`xv::syscall::sys_lstat`](sysfile.cpp.md#fn-a7bdc3d877fe1063)
- [`xv::syscall::sys_mkdir`](sysfile.cpp.md#fn-697c039ba51257c6)
- [`xv::syscall::sys_mknod`](sysfile.cpp.md#fn-a335916d7a484f15)
- [`xv::syscall::sys_mount`](sysfile.cpp.md#fn-23199bae69c6ad55)
- [`xv::syscall::sys_mountinfo`](sysfile.cpp.md#fn-52d39b8b47c88952)
- [`xv::syscall::sys_open`](sysfile.cpp.md#fn-db9a14b0a5f34f13)
- [`xv::syscall::sys_openpty`](sysfile.cpp.md#fn-6f37e9433a6bd907)
- [`xv::syscall::sys_pipe`](sysfile.cpp.md#fn-4fdcb85095f658e9)
- [`xv::syscall::sys_read`](sysfile.cpp.md#fn-fe1cdff590d53ee1)
- [`xv::syscall::sys_readlink`](sysfile.cpp.md#fn-8d50b39ab66db14f)
- [`xv::syscall::sys_rename`](sysfile.cpp.md#fn-7f137f88a1038fdf)
- [`xv::syscall::sys_stat`](sysfile.cpp.md#fn-eb860ac7c93040e6)
- [`xv::syscall::sys_symlink`](sysfile.cpp.md#fn-9a92c0afd955d412)
- [`xv::syscall::sys_sync`](sysfile.cpp.md#fn-f630890d41c7271a)
- [`xv::syscall::sys_umount`](sysfile.cpp.md#fn-103761d03e7af163)
- [`xv::syscall::sys_unlink`](sysfile.cpp.md#fn-4aa7dc56022e07b7)
- [`xv::syscall::sys_write`](sysfile.cpp.md#fn-dc225934cc4ef27f)

<a id="fn-4fdcb85095f658e9"></a>

## `xv::syscall::sys_pipe`

```doxygen
@fn uint64 xv::syscall::sys_pipe(void)
@brief 系统调用入口：pipe；从用户参数转入内核服务并返回 ABI 结果。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
@details 流程由 PipeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-6f37e9433a6bd907"></a>

## `xv::syscall::sys_openpty`

```doxygen
@fn uint64 xv::syscall::sys_openpty(void)
@brief 系统调用入口：openpty；从用户参数转入内核服务并返回 ABI 结果。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
@details 流程由 PtyOpenFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2940e0efab81491b"></a>

## `xv::syscall::sys_fcntl`

```doxygen
@fn uint64 xv::syscall::sys_fcntl(void)
@brief 系统调用入口：fcntl；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argfd` → `xv::fs::files` → `fcntl`。
@details 需要特别跟踪的条件：`command == F_SETFL`；`argfd(0, nullptr, &file) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::files().fcntl(file, command, static_cast<unsigned>(flags))`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-fe1cdff590d53ee1"></a>

## `xv::syscall::sys_read`

```doxygen
@fn uint64 xv::syscall::sys_read(void)
@brief 系统调用入口：读取；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `argument_integer` → `argfd` → `xv::fs::files` → `read`。
@details 需要特别跟踪的条件：`argfd(0, 0, &f) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::files().read(f, p, n)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-d063aa5f0fd12e38"></a>

## `xv::syscall::sys_exec`

```doxygen
@fn uint64 xv::syscall::sys_exec()
@brief 处理原有空环境 exec 系统调用。
@details 内核 SYS_exec 保持旧参数个数并委托 execute_image(false)。当前用户库 exec/execv 使用 execve 传入 environ，正常 shell 执行继承环境。
```

<a id="fn-c2ca80b2260b826c"></a>

## `xv::syscall::sys_fstat`

```doxygen
@fn uint64 xv::syscall::sys_fstat(void)
@brief 系统调用入口：fstat；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `argfd` → `xv::fs::files` → `stat`。
@details 需要特别跟踪的条件：`argfd(0, 0, &f) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::files().stat(f, st)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-5db6c22d9b99841b"></a>

## `xv::syscall::sys_chdir`

```doxygen
@fn uint64 xv::syscall::sys_chdir(void)
@brief 切换当前目录并验证搜索权限。
@details 有效身份解析路径，锁定最终节点确认目录类型和 X；成功转交引用给 cwd，失败归还引用，不要求目录 R。
@details 流程由 DirectoryFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-d76ff7559f643ffa"></a>

## `xv::syscall::sys_dup`

```doxygen
@fn uint64 xv::syscall::sys_dup(void)
@brief 系统调用入口：dup；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::current` → `files` → `duplicate`。
@return 各分支实际返回：`xv::process::current()->files().duplicate(descriptor)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-db9a14b0a5f34f13"></a>

## `xv::syscall::sys_open`

```doxygen
@fn uint64 xv::syscall::sys_open(void)
@brief 按访问模式和跟随策略建立打开对象。
@details O_CREATE 通过 VFS 处理悬空链接和 NOFOLLOW；普通查找拒绝最终未展开的链接。绑定 APPEND 标志，截断前清除特殊执行位，其余权限及失败引用归还保持原契约。
@details 流程由 OpenFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-dc225934cc4ef27f"></a>

## `xv::syscall::sys_write`

```doxygen
@fn uint64 xv::syscall::sys_write(void)
@brief 系统调用入口：写入；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `argument_integer` → `argfd` → `xv::fs::files` → `write`。
@details 需要特别跟踪的条件：`argfd(0, 0, &f) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::files().write(f, p, n)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-a335916d7a484f15"></a>

## `xv::syscall::sys_mknod`

```doxygen
@fn uint64 xv::syscall::sys_mknod(void)
@brief 系统调用入口：mknod；从用户参数转入内核服务并返回 ABI 结果。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
@details 流程由 CreateNodeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-4aa7dc56022e07b7"></a>

## `xv::syscall::sys_unlink`

```doxygen
@fn uint64 xv::syscall::sys_unlink(void)
@brief 系统调用入口：unlink；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_string` → `xv::fs::filesystem` → `unlink`。
@details 需要特别跟踪的条件：`argument_string(0, path, MAXPATH) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::filesystem().unlink(path)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-f0e4c3a2b61b65f2"></a>

## `xv::syscall::sys_link`

```doxygen
@fn uint64 xv::syscall::sys_link(void)
@brief 系统调用入口：link；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_string` → `xv::fs::filesystem` → `link`。
@details 需要特别跟踪的条件：`argument_string(0, old, MAXPATH) < 0 || argument_string(1, next, MAXPATH) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::filesystem().link(old, next)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-697c039ba51257c6"></a>

## `xv::syscall::sys_mkdir`

```doxygen
@fn uint64 xv::syscall::sys_mkdir(void)
@brief 系统调用入口：mkdir；从用户参数转入内核服务并返回 ABI 结果。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
@details 流程由 CreateNodeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-4692a7b4aafb1a42"></a>

## `xv::syscall::sys_close`

```doxygen
@fn uint64 xv::syscall::sys_close(void)
@brief 系统调用入口：close；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::current` → `files` → `close`。
@return 各分支实际返回：`xv::process::current()->files().close(descriptor) ? 0 : -1`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-f630890d41c7271a"></a>

## `xv::syscall::sys_sync`

```doxygen
@fn uint64 xv::syscall::sys_sync(void)
@brief 等待系统盘与可选数据盘完成提交。
@details 分别同步两个独立日志，屏障由日志和设备后端执行；不提供跨卷原子事务。
```

<a id="fn-868f8327fa5e0719"></a>

## `xv::syscall::sys_ioctl`

```doxygen
@fn uint64 xv::syscall::sys_ioctl(void)
@brief 系统调用入口：ioctl；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argument_address` → `argfd` → `xv::fs::files` → `ioctl`。
@details 需要特别跟踪的条件：`argfd(0, nullptr, &file) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`xv::fs::files().ioctl(file, static_cast<unsigned>(request), argument)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-7564cff018f6113b"></a>

## `xv::syscall::argfd`

```doxygen
@fn static int xv::syscall::argfd(int n, int *pfd, xv::fs::File **pf)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：取得并验证系统调用传入的文件描述符。
@param n 本次处理的数量上限。
@param pfd 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pf 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::current` → `files` → `get`。
@details 需要特别跟踪的条件：`fd < 0 || fd >= NOFILE || (f = xv::process::current()->files().get(fd)) == 0`；`pfd`；`pf`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e1520dac81970d65"></a>

## `xv::syscall::fdalloc`

```doxygen
@fn static int xv::syscall::fdalloc(xv::fs::File *f)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：在当前进程描述符表中安装一个文件引用。
@param f 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `p->files` → `install`。
@return 各分支实际返回：`p->files().install(f)`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-eb860ac7c93040e6"></a>

## `xv::syscall::sys_stat`

```doxygen
@fn uint64 xv::syscall::sys_stat()
@brief 按路径取得元数据而不打开文件。
@details 路径穿越仍要求目录 X，但读取最终节点属性不要求文件 R；持节点锁复制 Stat，归还引用后 copy_out，失败返回 -1。
@details 流程由 StatFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-58aee6d34f9fa96f"></a>

## `xv::syscall::sys_access`

```doxygen
@fn uint64 xv::syscall::sys_access()
@brief 按真实身份检查路径权限。
@details 验证 mode 仅含 R/W/X，完整路径搜索与最终节点均使用真实 UID/GID 和附加组。mode=0 只探测可搜索路径下的存在性；不授予后续打开能力。
@details 流程由 AccessFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-61a96d5c2d403c15"></a>

## `xv::syscall::sys_chown`

```doxygen
@fn uint64 xv::syscall::sys_chown()
@brief 处理属主和所属组修改系统调用。
@details 委托 attributes(true)，在节点锁内验证拥有者、组成员和 ID 范围；只读合成后端拒绝修改。
```

<a id="fn-94978fea0ce1a092"></a>

## `xv::syscall::sys_chmod`

```doxygen
@fn uint64 xv::syscall::sys_chmod()
@brief 处理文件权限修改系统调用。
@details 委托 attributes(false)，仅 root 或文件属主可设置 0000..01777；不支持 setuid/setgid 执行位，失败保持原元数据。
```

<a id="fn-2037b9e8e05eb105"></a>

## `xv::syscall::sys_execve`

```doxygen
@fn uint64 xv::syscall::sys_execve()
@brief 执行用户指定的参数与环境映像。
@details 委托 execute_image(true)，第三参数为 envp；nullptr 环境得到空环境。任何参数复制或映像装载失败都保留旧程序。
```

<a id="fn-9d6ef88240dcea18"></a>

## `xv::syscall::ArgumentVector::~ArgumentVector`

```doxygen
@fn xv::syscall::ArgumentVector::~ArgumentVector()
@brief 归还参数或环境向量的全部字符串页。
@details RAII 遍历零初始化槽，仅释放已分配页面，覆盖中途坏指针、超长字符串、容量超限和装载失败路径。
```

<a id="fn-29f8037673a75725"></a>

## `xv::syscall::ArgumentVector::copy`

```doxygen
@fn bool xv::syscall::ArgumentVector::copy(uint64 address)
@brief 有界复制用户字符串指针向量。
@details 拒绝空 argv 指针、指针算术溢出、无结束项和超过 MAXARG-1 的元素；每个字符串单独申请页并安全复制，部分失败由析构归还。当前环境使用相同 32 槽上限。
@details 流程由 CopyFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-17d77b950c1f0b08"></a>

## `xv::syscall::execute_image`

```doxygen
@fn uint64 xv::syscall::execute_image(bool with_environment)
@brief 统一复制参数及可选环境并执行映像。
@details 先复制路径和用户向量到 RAII 内核页，with_environment 决定是否读取第三参数。空 envp 视为空环境；全部复制成功才调用装载器，失败不改变旧地址空间。
@details 流程由 ExecuteImageFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2093ad83f06783f7"></a>

## `xv::syscall::attributes`

```doxygen
@fn uint64 xv::syscall::attributes(bool ownership)
@brief 检查并更新文件权限或属主。
@details 允许普通文件的特殊执行位，目录拒绝；非 root 的 setgid 需属于目标组。所有权变更清除 setid，仍要求 root 或文件属主且校验 ID 边界。
@details 流程由 AttributesFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-7f137f88a1038fdf"></a>

## `xv::syscall::sys_rename`

```doxygen
@fn uint64 xv::syscall::sys_rename()
@brief 校验用户路径并执行原子重命名。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-23199bae69c6ad55"></a>

## `xv::syscall::sys_mount`

```doxygen
@fn uint64 xv::syscall::sys_mount()
@brief 仅允许 root 挂载 RAM 或数据卷。
@details 精确接受 ramfs/xv6 两种类型，复制路径后调用对应服务；xv6 仅指已连接的数据卷，不接受任意设备或分区输入。
@details 流程由 MountFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-103761d03e7af163"></a>

## `xv::syscall::sys_umount`

```doxygen
@fn uint64 xv::syscall::sys_umount()
@brief 撤销挂载并等待相关日志完成。
@details 有效 UID 必须为零，VFS 先做引用与保护检查；成功后在事务外同步两个日志，避免持有自身日志预留时等待提交。
@details 流程由 UnmountFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-52d39b8b47c88952"></a>

## `xv::syscall::sys_mountinfo`

```doxygen
@fn uint64 xv::syscall::sys_mountinfo()
@brief 向用户复制一条挂载枚举记录。
@details 读取非负索引与用户地址，在内核栈准备固定大小 mount_info；VFS 返回记录后才 copy_out，枚举结束不触及用户缓冲。
@return 1 表示有记录，0 表示结束，-1 表示索引或复制错误。
@details 流程由 MountInfoFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-09832f54fc77b0ca"></a>

## `xv::syscall::sys_lseek`

```doxygen
@fn uint64 xv::syscall::sys_lseek()
@brief 读取偏移参数并转发到打开对象。
@details 按 64 位有符号位模式传递 offset，后端验证 origin 与范围；失败不改变共享偏移。
```

<a id="fn-ac0849d188c61352"></a>

## `xv::syscall::sys_ftruncate`

```doxygen
@fn uint64 xv::syscall::sys_ftruncate()
@brief 验证长度和可写描述符后调整文件。
@details 拒绝负数及超出当前上限的长度；文件表与后端进一步校验打开权限和普通文件类型。
```

<a id="fn-7cce73f1ff266383"></a>

## `xv::syscall::sys_fsync`

```doxygen
@fn uint64 xv::syscall::sys_fsync()
@brief 对有效描述符请求落盘。
@details 非法 fd 或对象不支持时返回 -1，持久 inode 后端等待日志并发送设备屏障。
```

<a id="fn-8d50b39ab66db14f"></a>

## `xv::syscall::sys_readlink`

```doxygen
@fn uint64 xv::syscall::sys_readlink()
@brief 复制链接节点保存的目标字节。
@details 事务中不跟随最终链接，锁定并检查 T_SYMLINK；按用户容量复制，不追加 NUL，返回复制字节数。
@details 流程由 ReadlinkFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-a7bdc3d877fe1063"></a>

## `xv::syscall::sys_lstat`

```doxygen
@fn uint64 xv::syscall::sys_lstat()
@brief 查询最终链接本身的元数据。
@details 复制用户路径，查找时禁止最终跟随；锁定节点生成内核 stat，再检查 copy_out 结果。
@details 流程由 StatFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-9a92c0afd955d412"></a>

## `xv::syscall::sys_symlink`

```doxygen
@fn uint64 xv::syscall::sys_symlink()
@brief 复制两个用户路径并创建链接。
@details 目标与安装路径均受 MAXPATH 限制；VFS 在事务内建立节点并处理写入失败回滚。
```

<a id="fn-5038a5cd17cfc6b3"></a>

## `xv::syscall::AccessFlow::assess`

```doxygen
@fn static void xv::syscall::AccessFlow::assess(Context &c) noexcept
@brief 按真实身份保存访问权限检查的业务返回值。
@details 使用真实身份进行路径查找和权限检查，检查后总是解锁归还 inode；Transaction 留在入口。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2813b911c455e413"></a>

## `xv::syscall::AccessFlow::lock`

```doxygen
@fn static void xv::syscall::AccessFlow::lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
@brief 锁住访问检查的 inode。
@details 使用真实身份进行路径查找和权限检查，检查后总是解锁归还 inode；Transaction 留在入口。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ce479401f632b549"></a>

## `xv::syscall::AccessFlow::lookup`

```doxygen
@fn static bool xv::syscall::AccessFlow::lookup(Context &c) noexcept
@brief 按真实身份搜索策略取得 inode。
@details 使用真实身份进行路径查找和权限检查，检查后总是解锁归还 inode；Transaction 留在入口。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-48a7b755baa05774"></a>

## `xv::syscall::AccessFlow::release`

```doxygen
@fn static void xv::syscall::AccessFlow::release(Context &c) noexcept
@brief 解锁归还访问检查的 inode。
@details 使用真实身份进行路径查找和权限检查，检查后总是解锁归还 inode；Transaction 留在入口。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-91233eb73b6dfcbc"></a>

## `xv::syscall::ArgumentVector::CopyFlow::advance`

```doxygen
@fn static void xv::syscall::ArgumentVector::CopyFlow::advance(Context &c) noexcept { ++c.index; }
@brief 推进向量索引以读取下一槽。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c54cda5406bd960d"></a>

## `xv::syscall::ArgumentVector::CopyFlow::allocate`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::allocate(Context &c) noexcept
@brief 为当前参数独立取得一页候选缓冲。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9034897e6826ce87"></a>

## `xv::syscall::ArgumentVector::CopyFlow::capacity`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::capacity(Context &c) noexcept { return c.index != MAXARG - 1; }
@brief 检查当前非空参数是否仍有终止槽容量。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-87463b48c9777268"></a>

## `xv::syscall::ArgumentVector::CopyFlow::copy_string`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::copy_string(Context &c) noexcept
@brief 复制当前参数字符串到其候选页。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-40e32de0e45ea40f"></a>

## `xv::syscall::ArgumentVector::CopyFlow::fetch`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::fetch(Context &c) noexcept
@brief 检查槽地址溢出并读取当前参数指针。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0f24651c03aaed27"></a>

## `xv::syscall::ArgumentVector::CopyFlow::present`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::present(Context &c) noexcept { return c.address != 0; }
@brief 检查参数向量地址是否非空。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-efe0ec5a57c4afae"></a>

## `xv::syscall::ArgumentVector::CopyFlow::success`

```doxygen
@fn static void xv::syscall::ArgumentVector::CopyFlow::success(Context &c) noexcept { c.result = true; }
@brief 记录参数向量复制完整终止。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-84defab755400773"></a>

## `xv::syscall::ArgumentVector::CopyFlow::terminated`

```doxygen
@fn static bool xv::syscall::ArgumentVector::CopyFlow::terminated(Context &c) noexcept { return c.argument == 0; }
@brief 检查是否已遇到参数终止空指针。
@details 每个非空参数都独立分配和复制，不合并重复地址；循环通过下一节点迭代，失败由原 ArgumentVector 析构回收已有候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-591fcbfd108388bb"></a>

## `xv::syscall::AttributesFlow::assess`

```doxygen
@fn static void xv::syscall::AttributesFlow::assess(Context &c) noexcept
@brief 按原凭据快照、所有权和组成员规则计算候选属性与合法性。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-211a3d44cf0d881d"></a>

## `xv::syscall::AttributesFlow::commit`

```doxygen
@fn static void xv::syscall::AttributesFlow::commit(Context &c) noexcept
@brief 提交已验证的 UID、GID 和权限模式。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-77562edf156a550b"></a>

## `xv::syscall::AttributesFlow::lock`

```doxygen
@fn static void xv::syscall::AttributesFlow::lock(Context &c) noexcept { xv::fs::filesystem().lock(c.node); }
@brief 锁住属性候选 inode。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-40b30b2f6625ab95"></a>

## `xv::syscall::AttributesFlow::lookup`

```doxygen
@fn static bool xv::syscall::AttributesFlow::lookup(Context &c) noexcept
@brief 查找待修改属性的 inode。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b63955f962ea33a9"></a>

## `xv::syscall::AttributesFlow::release`

```doxygen
@fn static void xv::syscall::AttributesFlow::release(Context &c) noexcept
@brief 解锁归还属性 inode。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7e454cabe9ec54aa"></a>

## `xv::syscall::AttributesFlow::valid`

```doxygen
@fn static bool xv::syscall::AttributesFlow::valid(Context &c) noexcept { return c.valid; }
@brief 检查候选属性是否满足全部原有规则。
@details chmod/chown 共用本图；凭据快照和标量权限策略在 inode 锁内计算，合法才提交，所有已锁分支汇合到解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d4cab267092dc934"></a>

## `xv::syscall::CreateNodeFlow::create`

```doxygen
@fn static bool xv::syscall::CreateNodeFlow::create(Context &c) noexcept
@brief 按入口提供的节点类型和设备号创建已锁 inode。
@details 目录和设备节点共用本图，Transaction 和参数读取仍在各自入口；创建成功取得的已锁 inode 必须解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a4fa459b6c0396ea"></a>

## `xv::syscall::CreateNodeFlow::release`

```doxygen
@fn static void xv::syscall::CreateNodeFlow::release(Context &c) noexcept
@brief 解锁归还新节点并记录成功。
@details 目录和设备节点共用本图，Transaction 和参数读取仍在各自入口；创建成功取得的已锁 inode 必须解锁归还。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8f0308f63d5de38e"></a>

## `xv::syscall::DirectoryFlow::commit`

```doxygen
@fn static void xv::syscall::DirectoryFlow::commit(Context &c) noexcept
@brief 解锁合格 inode 并把引用转给进程工作目录。
@details 保持查找、锁定、目录/搜索权限检查、解锁与工作目录引用转移的顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-58bfe15137cb3198"></a>

## `xv::syscall::DirectoryFlow::lock`

```doxygen
@fn static void xv::syscall::DirectoryFlow::lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
@brief 锁住候选 inode。
@details 保持查找、锁定、目录/搜索权限检查、解锁与工作目录引用转移的顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-aa0237f06b2f6349"></a>

## `xv::syscall::DirectoryFlow::lookup`

```doxygen
@fn static bool xv::syscall::DirectoryFlow::lookup(Context &c) noexcept
@brief 取得目标目录的候选 inode 引用。
@details 保持查找、锁定、目录/搜索权限检查、解锁与工作目录引用转移的顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6c070fca366f5106"></a>

## `xv::syscall::DirectoryFlow::release`

```doxygen
@fn static void xv::syscall::DirectoryFlow::release(Context &c) noexcept
@brief 解锁归还不合格的候选目录。
@details 保持查找、锁定、目录/搜索权限检查、解锁与工作目录引用转移的顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-64a45e417bcb835c"></a>

## `xv::syscall::DirectoryFlow::searchable`

```doxygen
@fn static bool xv::syscall::DirectoryFlow::searchable(Context &c) noexcept
@brief 检查候选是否为目录且具有搜索权限。
@details 保持查找、锁定、目录/搜索权限检查、解锁与工作目录引用转移的顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-65dcd7cb8075e930"></a>

## `xv::syscall::ExecuteImageFlow::arguments`

```doxygen
@fn static bool xv::syscall::ExecuteImageFlow::arguments(Context &c) noexcept { return c.argv.copy(c.arguments); }
@brief 复制本次 argv 向量。
@details argv/envp 仍为原入口局部 RAII 对象，失败按原析构顺序回收；公共 ArgumentVector 复制入口重复执行各次工作。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-20f444f4f32f8f8b"></a>

## `xv::syscall::ExecuteImageFlow::environment`

```doxygen
@fn static bool xv::syscall::ExecuteImageFlow::environment(Context &c) noexcept
@brief 环境向量存在时复制本次 envp。
@details argv/envp 仍为原入口局部 RAII 对象，失败按原析构顺序回收；公共 ArgumentVector 复制入口重复执行各次工作。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-420cbc4a3992ebd4"></a>

## `xv::syscall::ExecuteImageFlow::execute`

```doxygen
@fn static void xv::syscall::ExecuteImageFlow::execute(Context &c) noexcept
@brief 用完整候选参数调用原进程执行入口并保存结果。
@details argv/envp 仍为原入口局部 RAII 对象，失败按原析构顺序回收；公共 ArgumentVector 复制入口重复执行各次工作。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9f1803f1faaeda14"></a>

## `xv::syscall::MountFlow::disk`

```doxygen
@fn static bool xv::syscall::MountFlow::disk(Context &c) noexcept
@brief 检查请求类型是否为 xv6 磁盘文件系统。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-c5f23a8f6399d80f"></a>

## `xv::syscall::MountFlow::mount_disk`

```doxygen
@fn static void xv::syscall::MountFlow::mount_disk(Context &c) noexcept
@brief 使用MountDiskFlow挂载已选择磁盘后端，保留事务和失败回滚。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-cb43cf73881a02ed"></a>

## `xv::syscall::MountFlow::mount_ram`

```doxygen
@fn static void xv::syscall::MountFlow::mount_ram(Context &c) noexcept
@brief 执行原内存文件系统挂载并保存结果。
@details 权限及参数读取仍由入口按原顺序检查，图选择固定文件系统类型并交由原挂载业务执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1c62b766d8985c01"></a>

## `xv::syscall::MountFlow::ram`

```doxygen
@fn static bool xv::syscall::MountFlow::ram(Context &c) noexcept
@brief 检查请求类型是否为 ramfs。
@details 权限及参数读取仍由入口按原顺序检查，图选择固定文件系统类型并交由原挂载业务执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b43515f5dc5f0274"></a>

## `xv::syscall::MountInfoFlow::bad_copy`

```doxygen
@fn static void xv::syscall::MountInfoFlow::bad_copy(Context &c) noexcept { c.result = -1; }
@brief 记录挂载信息复制失败。
@details 先枚举，再仅在原结果非零时复制整个已清零记录；复制失败返回负一，其他结果保持原值。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-632ec74439b8aadd"></a>

## `xv::syscall::MountInfoFlow::copy`

```doxygen
@fn static bool xv::syscall::MountInfoFlow::copy(Context &c) noexcept
@brief 复制整个挂载记录给用户。
@details 先枚举，再仅在原结果非零时复制整个已清零记录；复制失败返回负一，其他结果保持原值。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-df7270b008da891b"></a>

## `xv::syscall::MountInfoFlow::describe`

```doxygen
@fn static void xv::syscall::MountInfoFlow::describe(Context &c) noexcept
@brief 按原索引枚举挂载路径与类型。
@details 先枚举，再仅在原结果非零时复制整个已清零记录；复制失败返回负一，其他结果保持原值。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0cff24bf64a3a2a6"></a>

## `xv::syscall::MountInfoFlow::has_result`

```doxygen
@fn static bool xv::syscall::MountInfoFlow::has_result(Context &c) noexcept { return c.result != 0; }
@brief 检查枚举返回值是否需要复制记录。
@details 先枚举，再仅在原结果非零时复制整个已清零记录；复制失败返回负一，其他结果保持原值。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f14af60a05e2c6c1"></a>

## `xv::syscall::OpenFlow::allocate`

```doxygen
@fn static bool xv::syscall::OpenFlow::allocate(Context &c) noexcept
@brief 取得候选打开文件引用。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b2dc915558ccaa53"></a>

## `xv::syscall::OpenFlow::bind`

```doxygen
@fn static bool xv::syscall::OpenFlow::bind(Context &c) noexcept
@brief 把已锁 inode 绑定到候选文件并保留原打开标志。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b1f61c339c709d16"></a>

## `xv::syscall::OpenFlow::create`

```doxygen
@fn static bool xv::syscall::OpenFlow::create(Context &c) noexcept
@brief 按原不跟随符号链接策略创建普通文件并取得已锁 inode。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-80f11e622ca85ecd"></a>

## `xv::syscall::OpenFlow::creating`

```doxygen
@fn static bool xv::syscall::OpenFlow::creating(Context &c) noexcept { return c.mode & O_CREATE; }
@brief 检查本次是否请求创建路径。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8b1a00b5e3f01a24"></a>

## `xv::syscall::OpenFlow::detach`

```doxygen
@fn static void xv::syscall::OpenFlow::detach(Context &c) noexcept
@brief 撤销已安装但绑定失败的描述符。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9c7aa5cc011e6f2d"></a>

## `xv::syscall::OpenFlow::existing_kind`

```doxygen
@fn static bool xv::syscall::OpenFlow::existing_kind(Context &c) noexcept
@brief 按原次序拒绝未跟随的符号链接和不合法目录打开模式。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fc4d2763f37b5724"></a>

## `xv::syscall::OpenFlow::install`

```doxygen
@fn static bool xv::syscall::OpenFlow::install(Context &c) noexcept
@brief 尝试在当前进程安装候选文件描述符。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-577aeb88257b0148"></a>

## `xv::syscall::OpenFlow::lock`

```doxygen
@fn static void xv::syscall::OpenFlow::lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
@brief 锁住查找取得的 inode。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9069da102898b0d4"></a>

## `xv::syscall::OpenFlow::lookup`

```doxygen
@fn static bool xv::syscall::OpenFlow::lookup(Context &c) noexcept
@brief 按原跟随策略查找 inode。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-37aa15003f34302b"></a>

## `xv::syscall::OpenFlow::permitted`

```doxygen
@fn static bool xv::syscall::OpenFlow::permitted(Context &c) noexcept
@brief 按原读写与截断访问掩码检查 inode 权限。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1a1b50a8b73bd583"></a>

## `xv::syscall::OpenFlow::release_file`

```doxygen
@fn static void xv::syscall::OpenFlow::release_file(Context &c) noexcept { xv::fs::files().release(c.file); }
@brief 归还未提交的候选文件引用。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8cdde0c43d301c71"></a>

## `xv::syscall::OpenFlow::release_inode`

```doxygen
@fn static void xv::syscall::OpenFlow::release_inode(Context &c) noexcept
@brief 解锁并归还未交付的 inode 引用。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d3b059ad234e3c53"></a>

## `xv::syscall::OpenFlow::success`

```doxygen
@fn static void xv::syscall::OpenFlow::success(Context &c) noexcept
@brief 解锁成功绑定的 inode 并记录描述符。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-77ff83112dd9e92f"></a>

## `xv::syscall::OpenFlow::truncate`

```doxygen
@fn static bool xv::syscall::OpenFlow::truncate(Context &c) noexcept
@brief 按原策略清除特殊权限位后截断 inode。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-9635ae189c28bdc2"></a>

## `xv::syscall::OpenFlow::truncating`

```doxygen
@fn static bool xv::syscall::OpenFlow::truncating(Context &c) noexcept
@brief 检查是否需要截断普通文件。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e32b35482a89e650"></a>

## `xv::syscall::OpenFlow::writable_mount`

```doxygen
@fn static bool xv::syscall::OpenFlow::writable_mount(Context &c) noexcept
@brief 检查普通文件的只读挂载限制。
@details Transaction 保留在原入口；create 返回已锁 inode，lookup 成功后加锁；失败按原顺序撤销描述符、文件引用和 inode，成功解锁并交出已绑定文件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-292b04e3efac928d"></a>

## `xv::syscall::PipeFlow::copy_reader`

```doxygen
@fn static bool xv::syscall::PipeFlow::copy_reader(Context &c) noexcept
@brief 先向用户输出读端描述符。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0eeea4b20ac2ee3c"></a>

## `xv::syscall::PipeFlow::copy_writer`

```doxygen
@fn static bool xv::syscall::PipeFlow::copy_writer(Context &c) noexcept
@brief 首个输出成功后输出写端描述符。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-51a0e3c4224e7cca"></a>

## `xv::syscall::PipeFlow::create`

```doxygen
@fn static bool xv::syscall::PipeFlow::create(Context &c) noexcept
@brief 创建持有读写文件引用的管道。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-412ae80dafe4e5f7"></a>

## `xv::syscall::PipeFlow::detach_both`

```doxygen
@fn static void xv::syscall::PipeFlow::detach_both(Context &c) noexcept
@brief 任一输出失败时按读写顺序撤销两个描述符。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8f80725730058215"></a>

## `xv::syscall::PipeFlow::detach_reader`

```doxygen
@fn static void xv::syscall::PipeFlow::detach_reader(Context &c) noexcept { c.process->files().take(c.first); }
@brief 写端安装失败时撤销已安装读端。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c025ee0e05f02ccc"></a>

## `xv::syscall::PipeFlow::install_reader`

```doxygen
@fn static bool xv::syscall::PipeFlow::install_reader(Context &c) noexcept
@brief 尝试安装读端描述符。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d42be175f81cf4d4"></a>

## `xv::syscall::PipeFlow::install_writer`

```doxygen
@fn static bool xv::syscall::PipeFlow::install_writer(Context &c) noexcept
@brief 在读端安装成功后尝试安装写端。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-58787419a89b2274"></a>

## `xv::syscall::PipeFlow::release`

```doxygen
@fn static void xv::syscall::PipeFlow::release(Context &c) noexcept
@brief 按原顺序归还读写两个文件引用。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-01f8c6d4a33e5367"></a>

## `xv::syscall::PipeFlow::success`

```doxygen
@fn static void xv::syscall::PipeFlow::success(Context &c) noexcept { c.result = 0; }
@brief 记录管道描述符交付成功。
@details 公共释放节点由全部安装和复制失败路径引用；分别复制两个描述符，第二次失败保留第一次用户写入但撤销两项安装。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-38541a888bfc07d7"></a>

## `xv::syscall::PtyOpenFlow::allocate`

```doxygen
@fn static void xv::syscall::PtyOpenFlow::allocate(Context &c) noexcept
@brief 按主端和从端次序取得两个候选文件引用。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a10b56192b5db31d"></a>

## `xv::syscall::PtyOpenFlow::bind`

```doxygen
@fn static void xv::syscall::PtyOpenFlow::bind(Context &c) noexcept
@brief 按原顺序把主从端点绑定到两个文件。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2c6e3eb6c886ed7b"></a>

## `xv::syscall::PtyOpenFlow::copy`

```doxygen
@fn static bool xv::syscall::PtyOpenFlow::copy(Context &c) noexcept
@brief 一次复制整个主从描述符数组给用户。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2db65bf386d1a4bb"></a>

## `xv::syscall::PtyOpenFlow::create`

```doxygen
@fn static bool xv::syscall::PtyOpenFlow::create(Context &c) noexcept
@brief 候选均存在时尝试创建主从 PTY。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0f9c8cbbedb9829f"></a>

## `xv::syscall::PtyOpenFlow::detach`

```doxygen
@fn static void xv::syscall::PtyOpenFlow::detach(Context &c) noexcept
@brief 按主从次序撤销实际已安装的描述符。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c3cdacf7e4457981"></a>

## `xv::syscall::PtyOpenFlow::install`

```doxygen
@fn static bool xv::syscall::PtyOpenFlow::install(Context &c) noexcept
@brief 取得当前文件表并依次尝试主从端安装。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3045ff1afe73e4ba"></a>

## `xv::syscall::PtyOpenFlow::release`

```doxygen
@fn static void xv::syscall::PtyOpenFlow::release(Context &c) noexcept
@brief 按主从次序归还存在的候选文件引用。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e922e669f63d689c"></a>

## `xv::syscall::PtyOpenFlow::success`

```doxygen
@fn static void xv::syscall::PtyOpenFlow::success(Context &c) noexcept { c.result = 0; }
@brief 记录主从描述符数组交付成功。
@details 两次文件分配都必须执行，候选完整才创建 PTY；绑定后安装主从端，失败先撤销已安装项再按主从顺序释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-deff4cbb5131c9fd"></a>

## `xv::syscall::ReadlinkFlow::lock`

```doxygen
@fn static void xv::syscall::ReadlinkFlow::lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
@brief 锁住目标 inode。
@details 保持不跟随的查找、锁定、符号链接类型判定、用户读取和必经的解锁归还顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c9563bce4f63f3d9"></a>

## `xv::syscall::ReadlinkFlow::lookup`

```doxygen
@fn static bool xv::syscall::ReadlinkFlow::lookup(Context &c) noexcept
@brief 不跟随符号链接查找目标 inode。
@details 保持不跟随的查找、锁定、符号链接类型判定、用户读取和必经的解锁归还顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e68cd13ce42653b3"></a>

## `xv::syscall::ReadlinkFlow::read`

```doxygen
@fn static void xv::syscall::ReadlinkFlow::read(Context &c) noexcept
@brief 按原长度直接把符号链接内容读取给用户。
@details 保持不跟随的查找、锁定、符号链接类型判定、用户读取和必经的解锁归还顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a8f4d5b237f66fbe"></a>

## `xv::syscall::ReadlinkFlow::release`

```doxygen
@fn static void xv::syscall::ReadlinkFlow::release(Context &c) noexcept
@brief 解锁归还符号链接检查的 inode。
@details 保持不跟随的查找、锁定、符号链接类型判定、用户读取和必经的解锁归还顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6944b1a6106c266e"></a>

## `xv::syscall::ReadlinkFlow::symlink`

```doxygen
@fn static bool xv::syscall::ReadlinkFlow::symlink(Context &c) noexcept { return c.inode->kind() == T_SYMLINK; }
@brief 检查目标是否确为符号链接。
@details 保持不跟随的查找、锁定、符号链接类型判定、用户读取和必经的解锁归还顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-579bc08c05755fd2"></a>

## `xv::syscall::StatFlow::copy`

```doxygen
@fn static void xv::syscall::StatFlow::copy(Context &c) noexcept
@brief 把统计快照复制给用户并保留原返回值。
@details stat/lstat 仅通过跟随策略区分，复制前先解锁归还 inode，原入口 Transaction 保持到复制结束。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c14adbd10597827e"></a>

## `xv::syscall::StatFlow::lock`

```doxygen
@fn static void xv::syscall::StatFlow::lock(Context &c) noexcept
@brief 准备零值统计缓冲并锁住 inode。
@details stat/lstat 仅通过跟随策略区分，复制前先解锁归还 inode，原入口 Transaction 保持到复制结束。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1ded90600992a4e9"></a>

## `xv::syscall::StatFlow::lookup`

```doxygen
@fn static bool xv::syscall::StatFlow::lookup(Context &c) noexcept
@brief 按本次跟随策略查找元数据 inode。
@details stat/lstat 仅通过跟随策略区分，复制前先解锁归还 inode，原入口 Transaction 保持到复制结束。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b4bd00ba23d091c6"></a>

## `xv::syscall::StatFlow::release`

```doxygen
@fn static void xv::syscall::StatFlow::release(Context &c) noexcept
@brief 解锁归还已取快照的 inode。
@details stat/lstat 仅通过跟随策略区分，复制前先解锁归还 inode，原入口 Transaction 保持到复制结束。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6593b020e780880e"></a>

## `xv::syscall::StatFlow::snapshot`

```doxygen
@fn static void xv::syscall::StatFlow::snapshot(Context &c) noexcept
@brief 在 inode 锁内读取统计快照。
@details stat/lstat 仅通过跟随策略区分，复制前先解锁归还 inode，原入口 Transaction 保持到复制结束。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8d5cf746df533b95"></a>

## `xv::syscall::UnmountFlow::removed`

```doxygen
@fn static bool xv::syscall::UnmountFlow::removed(Context &c) noexcept { return c.result == 0; }
@brief 检查卸载是否确实成功。
@details 仅卸载返回零时按原顺序同步主日志和存在的次日志，不改变失败结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ccfb906896eb2fdb"></a>

## `xv::syscall::UnmountFlow::sync`

```doxygen
@fn static void xv::syscall::UnmountFlow::sync(Context &) noexcept
@brief 成功卸载后按原次序同步主次日志。
@details 仅卸载返回零时按原顺序同步主日志和存在的次日志，不改变失败结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-56c8233a8d10a6f1"></a>

## `xv::syscall::UnmountFlow::unmount`

```doxygen
@fn static void xv::syscall::UnmountFlow::unmount(Context &c) noexcept
@brief 执行卸载并保存业务返回值。
@details 仅卸载返回零时按原顺序同步主日志和存在的次日志，不改变失败结果。 下一节点由图策略声明，业务结果与控制信号分离。
```
<a id="fn-fb210246732bc020"></a>

## `xv::syscall::sys_getdents64`

```doxygen
@fn uint64 xv::syscall::sys_getdents64()
@brief 实现 xv::syscall::sys_getdents64 的模块操作，按声明参数读写其封装状态。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-7643707c0f28dddb"></a>

## `xv::syscall::OpenFlow::unlock_bound`

```doxygen
@fn static void xv::syscall::OpenFlow::unlock_bound(Context &c) noexcept
@brief 流程动作或条件 unlock_bound；作用于当前Context，转移关系由所属图声明。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
