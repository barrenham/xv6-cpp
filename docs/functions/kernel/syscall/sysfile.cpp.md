# `kernel/syscall/sysfile.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysfile.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`xv.runtime`, `:arguments`, `:handlers`, `xv.fs`, `xv.process`, `xv.sync`, `xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::ArgumentVector::copy`](sysfile.cpp.md#fn-29f8037673a75725)
- [`xv::syscall::ArgumentVector::~ArgumentVector`](sysfile.cpp.md#fn-9d6ef88240dcea18)
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `argument_address` → `xv::fs::create_pipe` → `fdalloc` → `p->files` → `take` → `xv::fs::files` → `release` → `p->memory` → `copy_out`。
@details 需要特别跟踪的条件：`!xv::fs::create_pipe(&rf, &wf)`；`(fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0`；`fd0 >= 0`；`p->memory().copy_out(fdarray, &fd0, sizeof(fd0)) < 0 || p->memory().copy_out(fdarray + sizeof(fd0), &fd1, sizeof(fd1)) < 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-6f37e9433a6bd907"></a>

## `xv::syscall::sys_openpty`

```doxygen
@fn uint64 xv::syscall::sys_openpty(void)
@brief 系统调用入口：openpty；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `xv::fs::files` → `allocate` → `xv::drivers::create_pty` → `release` → `bind_endpoint` → `xv::process::current` → `files` → `table.install` → `memory` → `copy_out` → `table.take`。
@details 本函数可见的状态更新包括：`fds[2] = {-1, -1};`；`fds[0] = table.install(master_file);`；`fds[1] = table.install(slave_file);`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!master_file || !slave_file || !xv::drivers::create_pty(&master, &slave)`；`master_file`；`slave_file`；`fds[0] >= 0`；`fds[1] < 0 || xv::process::current()->memory().copy_out(address, fds, sizeof(fds)) < 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argument_string` → `xv::fs::filesystem` → `create` → `unlock_release`。
@details 需要特别跟踪的条件：`(argument_string(0, path, MAXPATH)) < 0 || (ip = xv::fs::filesystem().create(path, T_DEVICE, major, minor)) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_string` → `xv::fs::filesystem` → `create` → `unlock_release`。
@details 需要特别跟踪的条件：`argument_string(0, path, MAXPATH) < 0 || (ip = xv::fs::filesystem().create(path, T_DIR, 0, 0)) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
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
```

<a id="fn-58aee6d34f9fa96f"></a>

## `xv::syscall::sys_access`

```doxygen
@fn uint64 xv::syscall::sys_access()
@brief 按真实身份检查路径权限。
@details 验证 mode 仅含 R/W/X，完整路径搜索与最终节点均使用真实 UID/GID 和附加组。mode=0 只探测可搜索路径下的存在性；不授予后续打开能力。
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
```

<a id="fn-17d77b950c1f0b08"></a>

## `xv::syscall::execute_image`

```doxygen
@fn uint64 xv::syscall::execute_image(bool with_environment)
@brief 统一复制参数及可选环境并执行映像。
@details 先复制路径和用户向量到 RAII 内核页，with_environment 决定是否读取第三参数。空 envp 视为空环境；全部复制成功才调用装载器，失败不改变旧地址空间。
```

<a id="fn-2093ad83f06783f7"></a>

## `xv::syscall::attributes`

```doxygen
@fn uint64 xv::syscall::attributes(bool ownership)
@brief 检查并更新文件权限或属主。
@details 允许普通文件的特殊执行位，目录拒绝；非 root 的 setgid 需属于目标组。所有权变更清除 setid，仍要求 root 或文件属主且校验 ID 边界。
```

<a id="fn-7f137f88a1038fdf"></a>

## `xv::syscall::sys_rename`

```doxygen
@fn uint64 xv::syscall::sys_rename()
@brief 校验用户路径并执行原子重命名。
@details 复制两个非空 MAXPATH 路径，逐组件拒绝超过 DIRSIZ 的名称和尾部斜杠，然后转入 VFS。所有名字复制完成后才修改目录。
@return 0 表示成功；-1 表示路径复制、格式或 VFS 失败。
```

<a id="fn-23199bae69c6ad55"></a>

## `xv::syscall::sys_mount`

```doxygen
@fn uint64 xv::syscall::sys_mount()
@brief 仅允许 root 挂载 RAM 或数据卷。
@details 精确接受 ramfs/xv6 两种类型，复制路径后调用对应服务；xv6 仅指已连接的数据卷，不接受任意设备或分区输入。
```

<a id="fn-103761d03e7af163"></a>

## `xv::syscall::sys_umount`

```doxygen
@fn uint64 xv::syscall::sys_umount()
@brief 撤销挂载并等待相关日志完成。
@details 有效 UID 必须为零，VFS 先做引用与保护检查；成功后在事务外同步两个日志，避免持有自身日志预留时等待提交。
```

<a id="fn-52d39b8b47c88952"></a>

## `xv::syscall::sys_mountinfo`

```doxygen
@fn uint64 xv::syscall::sys_mountinfo()
@brief 向用户复制一条挂载枚举记录。
@details 读取非负索引与用户地址，在内核栈准备固定大小 mount_info；VFS 返回记录后才 copy_out，枚举结束不触及用户缓冲。
@return 1 表示有记录，0 表示结束，-1 表示索引或复制错误。
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
```

<a id="fn-a7bdc3d877fe1063"></a>

## `xv::syscall::sys_lstat`

```doxygen
@fn uint64 xv::syscall::sys_lstat()
@brief 查询最终链接本身的元数据。
@details 复制用户路径，查找时禁止最终跟随；锁定节点生成内核 stat，再检查 copy_out 结果。
```

<a id="fn-9a92c0afd955d412"></a>

## `xv::syscall::sys_symlink`

```doxygen
@fn uint64 xv::syscall::sys_symlink()
@brief 复制两个用户路径并创建链接。
@details 目标与安装路径均受 MAXPATH 限制；VFS 在事务内建立节点并处理写入失败回滚。
```
