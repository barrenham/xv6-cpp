# `kernel/process/exec/elf_loader.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/exec/elf_loader.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.print`, `xv.runtime`, `xv.fs`, `:elf_loader`, `:elf_format`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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
@fn bool xv::process::load_segment(AddressSpace &image, ExecutableFile &file, const proghdr &segment)
@brief 进程对象生命周期、组身份、调度、等待与信号：从可执行文件把一个 ELF 段装入候选映像。
@param image 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param segment 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`image.translate` → `xv::print::panic` → `file.read`。
@details 需要特别跟踪的条件：`!physical`；`count > PGSIZE`；`segment.off + offset < segment.off || !file.read(segment.off + offset, reinterpret_cast<void *>(physical), count)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
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
@fn int xv::process::ElfLoader::load(Process &process, const char *path, char *const *arguments, char *const *environment)
@brief 准备完整候选镜像后提交地址空间与执行身份。
@details 关闭可执行文件之前捕获 UID/GID/mode；所有 ELF、参数和环境准备成功后先提交镜像再应用凭据，任一前置失败不改变原进程身份。
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
