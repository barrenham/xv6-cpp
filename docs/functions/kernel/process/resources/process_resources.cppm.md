# `kernel/process/resources/process_resources.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/resources/process_resources.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:resources`。

直接模块依赖：`xv.fs`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::DescriptorTable::DescriptorTable`](process_resources.cppm.md#fn-e52a60bd1ba79c29)
- [`xv::process::DescriptorTable::DescriptorTable`](process_resources.cppm.md#fn-6bd60b86362d614c)
- [`xv::process::DescriptorTable::close`](process_resources.cpp.md#fn-315befdfc4ab2c6d)
- [`xv::process::DescriptorTable::close_all`](process_resources.cpp.md#fn-1a9397ea7f7cd7f3)
- [`xv::process::DescriptorTable::duplicate`](process_resources.cpp.md#fn-bf8e7f337214198f)
- [`xv::process::DescriptorTable::get`](process_resources.cpp.md#fn-5eee207e9f21abc9)
- [`xv::process::DescriptorTable::inherit_from`](process_resources.cpp.md#fn-067c825dc4aed5b6)
- [`xv::process::DescriptorTable::install`](process_resources.cpp.md#fn-9ff2e78e57ca3144)
- [`xv::process::DescriptorTable::operator=`](process_resources.cppm.md#fn-4bef4e5ee100877f)
- [`xv::process::DescriptorTable::take`](process_resources.cpp.md#fn-5f35dd820658a6cc)
- [`xv::process::WorkingDirectory::WorkingDirectory`](process_resources.cppm.md#fn-20def22537d3e99c)
- [`xv::process::WorkingDirectory::WorkingDirectory`](process_resources.cppm.md#fn-6c5a8b27ca49d496)
- [`xv::process::WorkingDirectory::clear`](process_resources.cpp.md#fn-3db3072ad2b9394a)
- [`xv::process::WorkingDirectory::duplicate`](process_resources.cpp.md#fn-4f397a6c89c47ea5)
- [`xv::process::WorkingDirectory::get`](process_resources.cppm.md#fn-40dc55094c1ea7f8)
- [`xv::process::WorkingDirectory::inherit_from`](process_resources.cpp.md#fn-1359f20c72f6c9b8)
- [`xv::process::WorkingDirectory::operator=`](process_resources.cppm.md#fn-11b3ecca9d0e0fd1)
- [`xv::process::WorkingDirectory::replace`](process_resources.cpp.md#fn-0c82862334d7f976)

<a id="fn-e52a60bd1ba79c29"></a>

## `xv::process::DescriptorTable::DescriptorTable`

```doxygen
@fn constexpr xv::process::DescriptorTable::DescriptorTable()
@brief 构造进程文件描述符表，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6bd60b86362d614c"></a>

## `xv::process::DescriptorTable::DescriptorTable`

```doxygen
@fn xv::process::DescriptorTable::DescriptorTable(const DescriptorTable &)
@brief 进程文件描述符表的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-4bef4e5ee100877f"></a>

## `xv::process::DescriptorTable::operator=`

```doxygen
@fn DescriptorTable &xv::process::DescriptorTable::operator=(const DescriptorTable &)
@brief 进程文件描述符表的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-20def22537d3e99c"></a>

## `xv::process::WorkingDirectory::WorkingDirectory`

```doxygen
@fn constexpr xv::process::WorkingDirectory::WorkingDirectory()
@brief 构造进程工作目录引用，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6c5a8b27ca49d496"></a>

## `xv::process::WorkingDirectory::WorkingDirectory`

```doxygen
@fn xv::process::WorkingDirectory::WorkingDirectory(const WorkingDirectory &)
@brief 进程工作目录引用的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-11b3ecca9d0e0fd1"></a>

## `xv::process::WorkingDirectory::operator=`

```doxygen
@fn WorkingDirectory &xv::process::WorkingDirectory::operator=(const WorkingDirectory &)
@brief 进程工作目录引用的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-40dc55094c1ea7f8"></a>

## `xv::process::WorkingDirectory::get`

```doxygen
@fn xv::fs::Inode *xv::process::WorkingDirectory::get() const
@brief 进程工作目录引用：读取或计算 `directory_`，返回当前查询结果。
@return 各分支实际返回：`directory_`。应结合上面的条件检查区分正常值、空值和错误值。
```
