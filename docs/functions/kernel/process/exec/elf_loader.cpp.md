# `kernel/process/exec/elf_loader.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/exec/elf_loader.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.print`, `xv.runtime`, `xv.fs`, `:elf_loader`, `:elf_format`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ElfLoader::LoadFlow::align_stack`](elf_loader.cpp.md#fn-edb070fe5a89c653)
- [`xv::process::ElfLoader::LoadFlow::argument`](elf_loader.cpp.md#fn-bc94e086cd75ba6d)
- [`xv::process::ElfLoader::LoadFlow::argument_array`](elf_loader.cpp.md#fn-d7d5f5b9efadf123)
- [`xv::process::ElfLoader::LoadFlow::argument_room`](elf_loader.cpp.md#fn-a8320c24dbca77a8)
- [`xv::process::ElfLoader::LoadFlow::argument_string`](elf_loader.cpp.md#fn-3770c9c4253b26a7)
- [`xv::process::ElfLoader::LoadFlow::close_file`](elf_loader.cpp.md#fn-ce4f50ddca9db51e)
- [`xv::process::ElfLoader::LoadFlow::commit`](elf_loader.cpp.md#fn-a902b387c4b6cfd6)
- [`xv::process::ElfLoader::LoadFlow::copy_segment`](elf_loader.cpp.md#fn-39751e338956f60d)
- [`xv::process::ElfLoader::LoadFlow::copy_stack`](elf_loader.cpp.md#fn-2451ac717d3d4e73)
- [`xv::process::ElfLoader::LoadFlow::create`](elf_loader.cpp.md#fn-854cb5c70d1f22c9)
- [`xv::process::ElfLoader::LoadFlow::environment`](elf_loader.cpp.md#fn-534cdc47b7fcac0f)
- [`xv::process::ElfLoader::LoadFlow::environment_address`](elf_loader.cpp.md#fn-b1c84704786a8a0c)
- [`xv::process::ElfLoader::LoadFlow::environment_array`](elf_loader.cpp.md#fn-2a9abc02890ee718)
- [`xv::process::ElfLoader::LoadFlow::environment_room`](elf_loader.cpp.md#fn-38a93bd887354f06)
- [`xv::process::ElfLoader::LoadFlow::environment_string`](elf_loader.cpp.md#fn-a8a4692563169c64)
- [`xv::process::ElfLoader::LoadFlow::guard`](elf_loader.cpp.md#fn-cd54ecd7f279730a)
- [`xv::process::ElfLoader::LoadFlow::header`](elf_loader.cpp.md#fn-10c3a1694fb9ea73)
- [`xv::process::ElfLoader::LoadFlow::loadable`](elf_loader.cpp.md#fn-f1c0a6ee276bfd10)
- [`xv::process::ElfLoader::LoadFlow::map_segment`](elf_loader.cpp.md#fn-f355f09bf69560bc)
- [`xv::process::ElfLoader::LoadFlow::map_stack`](elf_loader.cpp.md#fn-d9f7cbbce85b0a3c)
- [`xv::process::ElfLoader::LoadFlow::next_segment`](elf_loader.cpp.md#fn-e83830fcdc78fb5c)
- [`xv::process::ElfLoader::LoadFlow::read_segment`](elf_loader.cpp.md#fn-cdcd9d9d3b7b8d40)
- [`xv::process::ElfLoader::LoadFlow::segments`](elf_loader.cpp.md#fn-f06ab9599378026b)
- [`xv::process::ElfLoader::LoadFlow::space`](elf_loader.cpp.md#fn-e0a61803d803269a)
- [`xv::process::ElfLoader::LoadFlow::start_stack`](elf_loader.cpp.md#fn-6b711fbc26c8c60b)
- [`xv::process::ElfLoader::LoadFlow::store_argument`](elf_loader.cpp.md#fn-b26cb5a4678df203)
- [`xv::process::ElfLoader::LoadFlow::store_environment`](elf_loader.cpp.md#fn-4d99fa0ad6cc71ac)
- [`xv::process::ElfLoader::LoadFlow::valid_segment`](elf_loader.cpp.md#fn-fef4046207be1a02)
- [`xv::process::ElfLoader::load`](elf_loader.cpp.md#fn-7873c234f30790ee)
- [`xv::process::ExecutableFile::ExecutableFile`](elf_loader.cpp.md#fn-6e5b5d819ece94a1)
- [`xv::process::ExecutableFile::ExecutableFile`](elf_loader.cpp.md#fn-6e69081813061c93)
- [`xv::process::ExecutableFile::close`](elf_loader.cpp.md#fn-a12ac0a24317f534)
- [`xv::process::ExecutableFile::gid`](elf_loader.cpp.md#fn-35babcaa3cbccf81)
- [`xv::process::ExecutableFile::mode`](elf_loader.cpp.md#fn-03cff53d9fac314e)
- [`xv::process::ExecutableFile::operator bool`](elf_loader.cpp.md#fn-6ede2db410059966)
- [`xv::process::ExecutableFile::operator=`](elf_loader.cpp.md#fn-d853ede6e19ecd92)
- [`xv::process::ExecutableFile::read`](elf_loader.cpp.md#fn-5b23d6497d765137)
- [`xv::process::ExecutableFile::uid`](elf_loader.cpp.md#fn-930a35f280dcc8ee)
- [`xv::process::ExecutableFile::~ExecutableFile`](elf_loader.cpp.md#fn-567434fd12eba0bf)
- [`xv::process::PendingImage::PendingImage`](elf_loader.cpp.md#fn-a2994a1ee6aeaaf1)
- [`xv::process::PendingImage::PendingImage`](elf_loader.cpp.md#fn-b2cf62537f82e72a)
- [`xv::process::PendingImage::operator=`](elf_loader.cpp.md#fn-e1ba00541bfceede)
- [`xv::process::PendingImage::~PendingImage`](elf_loader.cpp.md#fn-8ed3391bb42667ba)
- [`xv::process::executable_loader`](elf_loader.cpp.md#fn-8a4db1fa5d5b6b13)
- [`xv::process::execute`](elf_loader.cpp.md#fn-51cf014f3d7bbc3b)
- [`xv::process::load_segment`](elf_loader.cpp.md#fn-3e3febc45083bc1e)
- [`xv::process::permissions`](elf_loader.cpp.md#fn-eb4a30b848de2d54)

<a id="fn-6e69081813061c93"></a>

## `xv::process::ExecutableFile::ExecutableFile`

```doxygen
@fn explicit xv::process::ExecutableFile::ExecutableFile(const char *path)
@brief 打开并验证可执行文件。
@details 路径搜索使用有效身份，持节点锁要求普通文件且满足 X；即使 root 也要求某个执行位。失败释放引用，不建立候选映像。
```

<a id="fn-567434fd12eba0bf"></a>

## `xv::process::ExecutableFile::~ExecutableFile`

```doxygen
@fn xv::process::ExecutableFile::~ExecutableFile()
@brief 结束可执行文件引用的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`close`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6e5b5d819ece94a1"></a>

## `xv::process::ExecutableFile::ExecutableFile`

```doxygen
@fn xv::process::ExecutableFile::ExecutableFile(const ExecutableFile &)
@brief 可执行文件引用的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-d853ede6e19ecd92"></a>

## `xv::process::ExecutableFile::operator=`

```doxygen
@fn ExecutableFile &xv::process::ExecutableFile::operator=(const ExecutableFile &)
@brief 可执行文件引用的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-6ede2db410059966"></a>

## `xv::process::ExecutableFile::operator bool`

```doxygen
@fn explicit xv::process::ExecutableFile::operator bool() const
@brief 检查可执行文件引用是否持有有效状态。
@return 各分支实际返回：`inode_ != nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5b23d6497d765137"></a>

## `xv::process::ExecutableFile::read`

```doxygen
@fn bool xv::process::ExecutableFile::read(unsigned long offset, void *destination, unsigned count)
@brief 可执行文件引用：读取已有数据并按接口返回读取结果。
@param offset 本次操作使用的 偏移 值。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::fs::filesystem` → `read`。
@details 需要特别跟踪的条件：`offset > ~0U`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`xv::fs::filesystem().read( inode_, 0, reinterpret_cast<unsigned long>(destination), static_cast<unsigned>(offset), count) == static_cast<int>(count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a12ac0a24317f534"></a>

## `xv::process::ExecutableFile::close`

```doxygen
@fn void xv::process::ExecutableFile::close()
@brief 可执行文件引用：关闭当前端点或释放文件关联。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::fs::filesystem` → `unlock_release` → `transaction_.finish`。
@details 本函数可见的状态更新包括：`inode_ = nullptr;`；`active_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!active_`；`inode_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8ed3391bb42667ba"></a>

## `xv::process::PendingImage::~PendingImage`

```doxygen
@fn xv::process::PendingImage::~PendingImage()
@brief 结束进程对象生命周期、组身份、调度、等待与信号的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`memory.reset`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a2994a1ee6aeaaf1"></a>

## `xv::process::PendingImage::PendingImage`

```doxygen
@fn xv::process::PendingImage::PendingImage()
@brief 构造进程对象生命周期、组身份、调度、等待与信号，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b2cf62537f82e72a"></a>

## `xv::process::PendingImage::PendingImage`

```doxygen
@fn xv::process::PendingImage::PendingImage(const PendingImage &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-e1ba00541bfceede"></a>

## `xv::process::PendingImage::operator=`

```doxygen
@fn PendingImage &xv::process::PendingImage::operator=(const PendingImage &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-eb4a30b848de2d54"></a>

## `xv::process::permissions`

```doxygen
@fn int xv::process::permissions(unsigned flags)
@brief 进程对象生命周期、组身份、调度、等待与信号：把 ELF 段标志转换为页表访问权限。
@param flags 接口标志位组合。
@details 需要特别跟踪的条件：`flags & ELF_PROG_FLAG_EXEC`；`flags & ELF_PROG_FLAG_WRITE`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`permission`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3e3febc45083bc1e"></a>

## `xv::process::load_segment`

```doxygen
@fn bool xv::process::load_segment(AddressSpace &image, ExecutableImage &file, const proghdr &segment)
@brief 经可执行数据源接口把有界 ELF 段复制到临时地址空间。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-8a4db1fa5d5b6b13"></a>

## `xv::process::executable_loader`

```doxygen
@fn ExecutableLoader &xv::process::executable_loader()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `loader`，返回当前查询结果。
@return 各分支实际返回：`loader`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7873c234f30790ee"></a>

## `xv::process::ElfLoader::load`

```doxygen
@fn int ElfLoader::xv::process::ElfLoader::load(Process &process, const char *path, char *const *arguments, char *const *environment)
@brief 准备带区域登记的候选镜像后提交执行状态。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-51cf014f3d7bbc3b"></a>

## `xv::process::execute`

```doxygen
@fn int xv::process::execute(const char *path, char *const *arguments, char *const *environment)
@brief 执行带显式环境的映像装载服务。
@details 委托当前进程的 ExecutableLoader；environment 为 nullptr 表示空环境，成功替换映像，失败返回 -1 且调用者原映像仍可继续。
```

<a id="fn-930a35f280dcc8ee"></a>

## `xv::process::ExecutableFile::uid`

```doxygen
@fn unsigned xv::process::ExecutableFile::uid() const
@brief 读取锁定可执行 inode 的属主 UID。
@details 供装载器在关闭文件前捕获特权执行身份；借用锁定 inode，不增加引用。
```

<a id="fn-35babcaa3cbccf81"></a>

## `xv::process::ExecutableFile::gid`

```doxygen
@fn unsigned xv::process::ExecutableFile::gid() const
@brief 读取锁定可执行 inode 的属组 GID。
@details 供装载器在关闭文件前捕获 setgid 身份，不从环境变量获得授权。
```

<a id="fn-03cff53d9fac314e"></a>

## `xv::process::ExecutableFile::mode`

```doxygen
@fn unsigned xv::process::ExecutableFile::mode() const
@brief 读取锁定可执行 inode 的权限位。
@details 装载器捕获 setid 位，失败路径不应用到当前进程。
```

<a id="fn-edb070fe5a89c653"></a>

## `xv::process::ElfLoader::LoadFlow::align_stack`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::align_stack(Context &c) noexcept
@brief 扣除入栈长度并向下对齐到十六字节。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-bc94e086cd75ba6d"></a>

## `xv::process::ElfLoader::LoadFlow::argument`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::argument(Context &c) noexcept
@brief 检查是否仍有参数字符串。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d7d5f5b9efadf123"></a>

## `xv::process::ElfLoader::LoadFlow::argument_array`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::argument_array(Context &c) noexcept
@brief 终止参数指针数组并准备将其复制到用户栈。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a8320c24dbca77a8"></a>

## `xv::process::ElfLoader::LoadFlow::argument_room`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::argument_room(Context &c) noexcept
@brief 检查参数指针数组是否仍可容纳终止项。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-3770c9c4253b26a7"></a>

## `xv::process::ElfLoader::LoadFlow::argument_string`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::argument_string(Context &c) noexcept
@brief 选择参数字符串并计算包含终止零的长度。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-ce4f50ddca9db51e"></a>

## `xv::process::ElfLoader::LoadFlow::close_file`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::close_file(Context &c) noexcept
@brief 保存文件属主和模式，并提前关闭文件与事务。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a902b387c4b6cfd6"></a>

## `xv::process::ElfLoader::LoadFlow::commit`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::commit(Context &c) noexcept
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-39751e338956f60d"></a>

## `xv::process::ElfLoader::LoadFlow::copy_segment`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::copy_segment(Context &c) noexcept
@brief 按页将文件段内容复制到候选映像。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-2451ac717d3d4e73"></a>

## `xv::process::ElfLoader::LoadFlow::copy_stack`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::copy_stack(Context &c) noexcept
@brief 验证对齐后边界并复制所选数据到用户栈。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-854cb5c70d1f22c9"></a>

## `xv::process::ElfLoader::LoadFlow::create`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::create(Context &c) noexcept
@brief 创建候选映像的特殊映射。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-534cdc47b7fcac0f"></a>

## `xv::process::ElfLoader::LoadFlow::environment`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::environment(Context &c) noexcept
@brief 检查是否仍有环境字符串。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-b1c84704786a8a0c"></a>

## `xv::process::ElfLoader::LoadFlow::environment_address`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::environment_address(Context &c) noexcept
@brief 保存用户环境指针数组地址。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-2a9abc02890ee718"></a>

## `xv::process::ElfLoader::LoadFlow::environment_array`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::environment_array(Context &c) noexcept
@brief 终止环境指针数组并准备将其复制到用户栈。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-38a93bd887354f06"></a>

## `xv::process::ElfLoader::LoadFlow::environment_room`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::environment_room(Context &c) noexcept
@brief 检查环境指针数组是否仍可容纳终止项。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a8a4692563169c64"></a>

## `xv::process::ElfLoader::LoadFlow::environment_string`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::environment_string(Context &c) noexcept
@brief 选择环境字符串并保留原长度计算规则。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-cd54ecd7f279730a"></a>

## `xv::process::ElfLoader::LoadFlow::guard`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::guard(Context &c) noexcept
@brief 将用户栈底部一页改为保护页。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-10c3a1694fb9ea73"></a>

## `xv::process::ElfLoader::LoadFlow::header`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::header(Context &c) noexcept
@brief 读取并校验可执行文件头。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f1c0a6ee276bfd10"></a>

## `xv::process::ElfLoader::LoadFlow::loadable`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::loadable(Context &c) noexcept
@brief 检查当前程序头是否为可装载段。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f355f09bf69560bc"></a>

## `xv::process::ElfLoader::LoadFlow::map_segment`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::map_segment(Context &c) noexcept
@brief 为当前段扩展候选地址空间并应用段权限。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d9f7cbbce85b0a3c"></a>

## `xv::process::ElfLoader::LoadFlow::map_stack`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::map_stack(Context &c) noexcept
@brief 按页对齐映像末尾并分配用户栈区域。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e83830fcdc78fb5c"></a>

## `xv::process::ElfLoader::LoadFlow::next_segment`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::next_segment(Context &c) noexcept
@brief 推进程序头索引。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-cdcd9d9d3b7b8d40"></a>

## `xv::process::ElfLoader::LoadFlow::read_segment`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::read_segment(Context &c) noexcept
@brief 验证程序头偏移未溢出并读取本次程序头。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f06ab9599378026b"></a>

## `xv::process::ElfLoader::LoadFlow::segments`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::segments(Context &c) noexcept
@brief 检查是否还有程序头需要读取。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e0a61803d803269a"></a>

## `xv::process::ElfLoader::LoadFlow::space`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::space(Context &c) noexcept
@brief 检查当前入栈数据大小是否落在剩余栈空间内。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-6b711fbc26c8c60b"></a>

## `xv::process::ElfLoader::LoadFlow::start_stack`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::start_stack(Context &c) noexcept
@brief 初始化栈顶与可用栈底边界。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-b26cb5a4678df203"></a>

## `xv::process::ElfLoader::LoadFlow::store_argument`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::store_argument(Context &c) noexcept
@brief 记录参数字符串的用户地址并推进索引。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-4d99fa0ad6cc71ac"></a>

## `xv::process::ElfLoader::LoadFlow::store_environment`

```doxygen
@fn static void xv::process::ElfLoader::LoadFlow::store_environment(Context &c) noexcept
@brief 记录环境字符串的用户地址并推进索引。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-fef4046207be1a02"></a>

## `xv::process::ElfLoader::LoadFlow::valid_segment`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::valid_segment(Context &c) noexcept
@brief 验证内存大小、虚拟页对齐和段末地址未溢出。
@details 候选映像和可执行文件保留在 load 的词法作用域中；失败释放候选，成功交换后才回收旧映像。四个入栈调用共用 PushBounds 子流程，return_ 返回调用位置，失败 stop 结束整次装载。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
<a id="fn-aab803e89745a022"></a>

## `xv::process::ElfLoader::LoadFlow::snapshot`

```doxygen
@fn static bool xv::process::ElfLoader::LoadFlow::snapshot(Context &c) noexcept
@brief 流程动作或条件 snapshot；作用于当前Context，转移关系由所属图声明。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

- [`xv::process::ElfLoader::LoadFlow::snapshot`](elf_loader.cpp.md#fn-aab803e89745a022)

<a id="fn-84a204ae66249b72"></a>

## `xv::process::ElfLoader::load_memory`

```doxygen
@fn int xv::process::ElfLoader::load_memory(Process &process, const void *bytes, unsigned long length, const char *path, char *const *arguments, char *const *environment)
@brief 从有界内存源执行共用 ELF 装载和映像提交。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-45acaf9a6e115a7a"></a>

## `xv::process::ExecutableImage::close`

```doxygen
@fn virtual void xv::process::ExecutableImage::close() = 0
@brief 结束可执行数据源的装载访问。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-85cecc4e5527c7fc"></a>

## `xv::process::ExecutableImage::gid`

```doxygen
@fn virtual unsigned xv::process::ExecutableImage::gid() const = 0
@brief 查询映像属组 GID，供装载提交使用。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-65fa1cdce6cb24a0"></a>

## `xv::process::ExecutableImage::mode`

```doxygen
@fn virtual unsigned xv::process::ExecutableImage::mode() const = 0
@brief 查询映像执行模式，供装载提交使用。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-efc30d35adf392d0"></a>

## `xv::process::ExecutableImage::operator bool`

```doxygen
@fn virtual explicit xv::process::ExecutableImage::operator bool() const = 0
@brief 检查可执行数据源是否有效。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-7f82ec358bb5c3c2"></a>

## `xv::process::ExecutableImage::read`

```doxygen
@fn virtual bool xv::process::ExecutableImage::read(unsigned long offset, void *destination, unsigned count) = 0
@brief 从可执行数据源读取指定范围。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-60f787e217c99fdc"></a>

## `xv::process::ExecutableImage::uid`

```doxygen
@fn virtual unsigned xv::process::ExecutableImage::uid() const = 0
@brief 查询映像属主 UID，供装载提交使用。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-baf48dbc268f3cb2"></a>

## `xv::process::ExecutableImage::~ExecutableImage`

```doxygen
@fn xv::process::ExecutableImage::~ExecutableImage() = default
@brief 销毁可执行数据源接口。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-908e2c50b7ea515c"></a>

## `xv::process::ExecutableMemory::ExecutableMemory`

```doxygen
@fn xv::process::ExecutableMemory::ExecutableMemory(const void *bytes, unsigned long count)
@brief 绑定已预留引导字节的指针与完整长度。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-e4e727acdd09597e"></a>

## `xv::process::ExecutableMemory::close`

```doxygen
@fn void xv::process::ExecutableMemory::close() override
@brief 结束内存源访问，预留引导页面仍由内核保有。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-2122b5d6811a8e8a"></a>

## `xv::process::ExecutableMemory::gid`

```doxygen
@fn unsigned xv::process::ExecutableMemory::gid() const override
@brief 为初始内存映像提供 root GID。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-1cddd50d92901673"></a>

## `xv::process::ExecutableMemory::mode`

```doxygen
@fn unsigned xv::process::ExecutableMemory::mode() const override
@brief 为初始内存映像提供 0755 执行模式。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-6e3ef5856a06c0c2"></a>

## `xv::process::ExecutableMemory::operator bool`

```doxygen
@fn explicit xv::process::ExecutableMemory::operator bool() const override
@brief 检查有界内存映像是否存在。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-46348b779500d5ac"></a>

## `xv::process::ExecutableMemory::read`

```doxygen
@fn bool xv::process::ExecutableMemory::read(unsigned long offset, void *destination, unsigned count) override
@brief 边界校验后从预留引导镜像复制指定字节。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

<a id="fn-f82035903b098fa0"></a>

## `xv::process::ExecutableMemory::uid`

```doxygen
@fn unsigned xv::process::ExecutableMemory::uid() const override
@brief 为初始内存映像提供 root UID。
@details 文件和内存引导载荷共用 ELF 校验、段装载及地址空间提交协议。内存源严格检查 offset/count，不查路径、不获取文件系统事务，并使用 root/0755 初始身份；路径源继续检查 inode 类型、执行权限与所有权。任何装载失败保留旧进程映像并释放临时资源。
```

- [`xv::process::ElfLoader::load_memory`](elf_loader.cpp.md#fn-84a204ae66249b72)

- [`xv::process::ExecutableImage::close`](elf_loader.cpp.md#fn-45acaf9a6e115a7a)

- [`xv::process::ExecutableImage::gid`](elf_loader.cpp.md#fn-85cecc4e5527c7fc)

- [`xv::process::ExecutableImage::mode`](elf_loader.cpp.md#fn-65fa1cdce6cb24a0)

- [`xv::process::ExecutableImage::operator bool`](elf_loader.cpp.md#fn-efc30d35adf392d0)

- [`xv::process::ExecutableImage::read`](elf_loader.cpp.md#fn-7f82ec358bb5c3c2)

- [`xv::process::ExecutableImage::uid`](elf_loader.cpp.md#fn-60f787e217c99fdc)

- [`xv::process::ExecutableImage::~ExecutableImage`](elf_loader.cpp.md#fn-baf48dbc268f3cb2)

- [`xv::process::ExecutableMemory::ExecutableMemory`](elf_loader.cpp.md#fn-908e2c50b7ea515c)

- [`xv::process::ExecutableMemory::close`](elf_loader.cpp.md#fn-e4e727acdd09597e)

- [`xv::process::ExecutableMemory::gid`](elf_loader.cpp.md#fn-2122b5d6811a8e8a)

- [`xv::process::ExecutableMemory::mode`](elf_loader.cpp.md#fn-1cddd50d92901673)

- [`xv::process::ExecutableMemory::operator bool`](elf_loader.cpp.md#fn-6e3ef5856a06c0c2)

- [`xv::process::ExecutableMemory::read`](elf_loader.cpp.md#fn-46348b779500d5ac)

- [`xv::process::ExecutableMemory::uid`](elf_loader.cpp.md#fn-f82035903b098fa0)
