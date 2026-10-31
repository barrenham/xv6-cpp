# `kernel/process/process.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../kernel/process/process.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:process`。

直接模块依赖：`:registers`, `:resources`, `:address_space`, `xv.sync`, `:identity`, `:signals`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::Process`](process.cppm.md#fn-4994347ef002a2f9)
- [`xv::process::Process::Process`](process.cppm.md#fn-8f111832cbfaa884)
- [`xv::process::Process::begin_system_call`](signals/user_signals.cpp.md#fn-58ff4ed2bf6d42b0)
- [`xv::process::Process::create_image`](lifecycle/process.cpp.md#fn-9d91050686d2abd0)
- [`xv::process::Process::credentials`](process.cppm.md#fn-49caab07f1a46655)
- [`xv::process::Process::credentials`](process.cppm.md#fn-96836ea87a81f92a)
- [`xv::process::Process::debug_name`](process.cppm.md#fn-66a9229e537d779c)
- [`xv::process::Process::directory`](process.cppm.md#fn-0c3708fa5ad3c9e2)
- [`xv::process::Process::enter_signal_handler`](signals/user_signals.cpp.md#fn-a4af1623ff74134d)
- [`xv::process::Process::files`](process.cppm.md#fn-fcd122e5f3c16a30)
- [`xv::process::Process::finish_system_call`](signals/user_signals.cpp.md#fn-2c3b9b20d500a9c5)
- [`xv::process::Process::id`](process.cppm.md#fn-f83a37175d9f826f)
- [`xv::process::Process::is_killed`](signals/process_signals.cpp.md#fn-f4265352809f85a0)
- [`xv::process::Process::kernel_stack`](process.cppm.md#fn-d6f748dc0645b5b1)
- [`xv::process::Process::memory`](process.cppm.md#fn-b5b32a874cb6d4f2)
- [`xv::process::Process::memory`](process.cppm.md#fn-905b4ce55f034885)
- [`xv::process::Process::operator=`](process.cppm.md#fn-064e199494446673)
- [`xv::process::Process::pin_cpu`](arch/cpu_affinity.cpp.md#fn-7cb6f1f52058eb7d)
- [`xv::process::Process::registers`](process.cppm.md#fn-15760d1db617ef9a)
- [`xv::process::Process::request_kill`](signals/process_signals.cpp.md#fn-d56b75df7731853a)
- [`xv::process::Process::set_name`](lifecycle/process.cpp.md#fn-7970dbfee688fb06)
- [`xv::process::Process::signal_action`](signals/user_signals.cpp.md#fn-bc793784f39c34ed)
- [`xv::process::Process::signal_mask`](signals/user_signals.cpp.md#fn-ef5ff2ec7facd8e3)
- [`xv::process::Process::signal_pending`](signals/user_signals.cpp.md#fn-ea7534a5b6b93ae7)
- [`xv::process::Process::signal_return`](signals/user_signals.cpp.md#fn-5c2916a90196dfd3)
- [`xv::process::Process::signal_suspend`](signals/user_signals.cpp.md#fn-824959f8e4a5aee2)
- [`xv::process::Process::stop_requested`](process.cppm.md#fn-5e15db3a4f0c6cdb)
- [`xv::process::Process::termination_status`](signals/process_signals.cpp.md#fn-5e63eeb4721e2fa0)
- [`xv::process::Process::~Process`](process.cppm.md#fn-1a52c83822d8ab9a)
- [`xv::process::cpu_id`](arch/cpu.cpp.md#fn-38c0b9f44adde749)
- [`xv::process::current`](arch/cpu.cpp.md#fn-3674a9cba9c27758)
- [`xv::process::current_cpu`](arch/cpu.cpp.md#fn-eaa0d9fcd809861d)

<a id="fn-1a52c83822d8ab9a"></a>

## `xv::process::Process::~Process`

```doxygen
@fn xv::process::Process::~Process()
@brief 结束进程描述符的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8f111832cbfaa884"></a>

## `xv::process::Process::Process`

```doxygen
@fn xv::process::Process::Process(const Process &)
@brief 进程描述符的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-064e199494446673"></a>

## `xv::process::Process::operator=`

```doxygen
@fn Process &xv::process::Process::operator=(const Process &)
@brief 进程描述符的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-f83a37175d9f826f"></a>

## `xv::process::Process::id`

```doxygen
@fn int xv::process::Process::id() const
@brief 进程描述符：读取或计算 `pid`，返回当前查询结果。
@return 各分支实际返回：`pid`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-66a9229e537d779c"></a>

## `xv::process::Process::debug_name`

```doxygen
@fn const char *xv::process::Process::debug_name() const
@brief 进程描述符：读取或计算 `name`，返回当前查询结果。
@return 各分支实际返回：`name`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5e15db3a4f0c6cdb"></a>

## `xv::process::Process::stop_requested`

```doxygen
@fn inline bool xv::process::Process::stop_requested() const
@brief 进程描述符：读取或计算 `pending_signals_.stopping_signal() != 0`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pending_signals_.stopping_signal`。
@return 各分支实际返回：`pending_signals_.stopping_signal() != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d6f748dc0645b5b1"></a>

## `xv::process::Process::kernel_stack`

```doxygen
@fn unsigned long xv::process::Process::kernel_stack() const
@brief 进程描述符：读取或计算 `kstack`，返回当前查询结果。
@return 各分支实际返回：`kstack`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-15760d1db617ef9a"></a>

## `xv::process::Process::registers`

```doxygen
@fn TrapFrame *xv::process::Process::registers()
@brief 进程描述符：读取或计算 `trapframe`，返回当前查询结果。
@return 各分支实际返回：`trapframe`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b5b32a874cb6d4f2"></a>

## `xv::process::Process::memory`

```doxygen
@fn AddressSpace &xv::process::Process::memory()
@brief 进程描述符：读取或计算 `memory_`，返回当前查询结果。
@return 各分支实际返回：`memory_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-905b4ce55f034885"></a>

## `xv::process::Process::memory`

```doxygen
@fn const AddressSpace &xv::process::Process::memory() const
@brief 进程描述符：读取或计算 `memory_`，返回当前查询结果。
@return 各分支实际返回：`memory_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fcd122e5f3c16a30"></a>

## `xv::process::Process::files`

```doxygen
@fn DescriptorTable &xv::process::Process::files()
@brief 进程描述符：读取或计算 `descriptors_`，返回当前查询结果。
@return 各分支实际返回：`descriptors_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0c3708fa5ad3c9e2"></a>

## `xv::process::Process::directory`

```doxygen
@fn WorkingDirectory &xv::process::Process::directory()
@brief 进程描述符：读取或计算 `directory_`，返回当前查询结果。
@return 各分支实际返回：`directory_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4994347ef002a2f9"></a>

## `xv::process::Process::Process`

```doxygen
@fn constexpr xv::process::Process::Process()
@brief 构造进程描述符，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
<a id="fn-49caab07f1a46655"></a>

## `xv::process::Process::credentials`

```doxygen
@fn Credentials &xv::process::Process::credentials()
@brief 取得进程只读凭据控制对象。
@details 返回 const 借用，通常调用 snapshot 取得一致值；不能跨进程寿命保留引用。
```

<a id="fn-96836ea87a81f92a"></a>

## `xv::process::Process::credentials`

```doxygen
@fn const Credentials &xv::process::Process::credentials() const
@brief 取得进程只读凭据控制对象。
@details 返回 const 借用，通常调用 snapshot 取得一致值；不能跨进程寿命保留引用。
```
