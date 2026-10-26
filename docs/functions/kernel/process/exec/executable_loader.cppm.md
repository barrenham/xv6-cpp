# `kernel/process/exec/executable_loader.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/exec/executable_loader.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:executable`。

直接模块依赖：`:process`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::executable_loader`](elf_loader.cpp.md#fn-8a4db1fa5d5b6b13)
- [`xv::process::execute`](elf_loader.cpp.md#fn-51cf014f3d7bbc3b)
- [`xv::process::ExecutableLoader::ExecutableLoader`](executable_loader.cppm.md#fn-bddc2a517ac5e216)
- [`xv::process::ExecutableLoader::ExecutableLoader`](executable_loader.cppm.md#fn-5855c0a2fb1c0f54)
- [`xv::process::ExecutableLoader::load`](executable_loader.cppm.md#fn-38c2b8267e0e0284)
- [`xv::process::ExecutableLoader::name`](executable_loader.cppm.md#fn-d209fc69858331be)
- [`xv::process::ExecutableLoader::operator=`](executable_loader.cppm.md#fn-6a14f53f908da8f8)
- [`xv::process::ExecutableLoader::~ExecutableLoader`](executable_loader.cppm.md#fn-23d934078b39e374)

<a id="fn-5855c0a2fb1c0f54"></a>

## `xv::process::ExecutableLoader::ExecutableLoader`

```doxygen
@fn xv::process::ExecutableLoader::ExecutableLoader(const ExecutableLoader &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-6a14f53f908da8f8"></a>

## `xv::process::ExecutableLoader::operator=`

```doxygen
@fn ExecutableLoader &xv::process::ExecutableLoader::operator=(const ExecutableLoader &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-d209fc69858331be"></a>

## `xv::process::ExecutableLoader::name`

```doxygen
@fn virtual const char *xv::process::ExecutableLoader::name() const
@brief 进程对象生命周期、组身份、调度、等待与信号：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-38c2b8267e0e0284"></a>

## `xv::process::ExecutableLoader::load`

```doxygen
@fn virtual int xv::process::ExecutableLoader::load(Process &process, const char *path, char *const *arguments)
@brief 进程对象生命周期、组身份、调度、等待与信号：校验并装载候选可执行映像。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param path 待解析的零结尾路径字符串。
@param arguments 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (Process &, const char *, char *const *)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-bddc2a517ac5e216"></a>

## `xv::process::ExecutableLoader::ExecutableLoader`

```doxygen
@fn constexpr xv::process::ExecutableLoader::ExecutableLoader()
@brief 构造进程对象生命周期、组身份、调度、等待与信号，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-23d934078b39e374"></a>

## `xv::process::ExecutableLoader::~ExecutableLoader`

```doxygen
@fn xv::process::ExecutableLoader::~ExecutableLoader()
@brief 结束进程对象生命周期、组身份、调度、等待与信号的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
