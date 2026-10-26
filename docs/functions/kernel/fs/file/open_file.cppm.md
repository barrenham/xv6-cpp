# `kernel/fs/file/open_file.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/open_file.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:file`。

直接模块依赖：`:filesystem`, `xv.drivers`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::create_pipe`](../pipe/pipe_factory.cpp.md#fn-6abd7f37e1481270)
- [`xv::fs::devices`](../service.cpp.md#fn-94c869e5070c351f)
- [`xv::fs::files`](../service.cpp.md#fn-f6456e38e2f03d00)
- [`xv::fs::DeviceRegistry::DeviceRegistry`](open_file.cppm.md#fn-22ab25588e11a366)
- [`xv::fs::DeviceRegistry::attach`](open_file.cpp.md#fn-bda8b34bb94e637a)
- [`xv::fs::DeviceRegistry::get`](open_file.cpp.md#fn-ad2454960d73ee76)
- [`xv::fs::File::File`](open_file.cppm.md#fn-25a003ad25b12bc4)
- [`xv::fs::File::File`](open_file.cppm.md#fn-1999327836ac2dff)
- [`xv::fs::File::nonblocking`](open_file.cpp.md#fn-6ae1d9a30631a953)
- [`xv::fs::File::operator=`](open_file.cppm.md#fn-23c65384bb5187b1)
- [`xv::fs::OpenFileTable::OpenFileTable`](open_file.cppm.md#fn-1ac19dfbfd9e17f1)
- [`xv::fs::OpenFileTable::allocate`](open_file.cpp.md#fn-319840ebccd17589)
- [`xv::fs::OpenFileTable::bind_endpoint`](open_file.cpp.md#fn-71d5734b8a7caee5)
- [`xv::fs::OpenFileTable::bind_inode`](open_file.cpp.md#fn-dd0a9f0a843218e4)
- [`xv::fs::OpenFileTable::bind_object`](open_file.cpp.md#fn-5b6c8b6fa35ede20)
- [`xv::fs::OpenFileTable::bind_pipe`](open_file.cpp.md#fn-28ec255ef808a0a2)
- [`xv::fs::OpenFileTable::configure`](open_file.cpp.md#fn-34475d43f15a5471)
- [`xv::fs::OpenFileTable::fcntl`](open_file.cpp.md#fn-3a55abe43cfd3ddc)
- [`xv::fs::OpenFileTable::initialize`](open_file.cpp.md#fn-6b5a5967a8ffe320)
- [`xv::fs::OpenFileTable::ioctl`](open_file.cpp.md#fn-227fb8963ea0dda0)
- [`xv::fs::OpenFileTable::object`](open_file.cpp.md#fn-2e5a0e8ff1089c2a)
- [`xv::fs::OpenFileTable::poll`](open_file.cpp.md#fn-1565f5ef091ec77d)
- [`xv::fs::OpenFileTable::read`](open_file.cpp.md#fn-ca0e6e5ab12e2355)
- [`xv::fs::OpenFileTable::release`](open_file.cpp.md#fn-68d546d0ff9f6adb)
- [`xv::fs::OpenFileTable::retain`](open_file.cpp.md#fn-5dc21f043fc5419d)
- [`xv::fs::OpenFileTable::stat`](open_file.cpp.md#fn-bc87515094e34bf0)
- [`xv::fs::OpenFileTable::write`](open_file.cpp.md#fn-0115ff202f5e535d)

<a id="fn-25a003ad25b12bc4"></a>

## `xv::fs::File::File`

```doxygen
@fn constexpr xv::fs::File::File()
@brief 构造打开文件描述，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1999327836ac2dff"></a>

## `xv::fs::File::File`

```doxygen
@fn xv::fs::File::File(const File &)
@brief 打开文件描述的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-23c65384bb5187b1"></a>

## `xv::fs::File::operator=`

```doxygen
@fn File &xv::fs::File::operator=(const File &)
@brief 打开文件描述的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-1ac19dfbfd9e17f1"></a>

## `xv::fs::OpenFileTable::OpenFileTable`

```doxygen
@fn constexpr xv::fs::OpenFileTable::OpenFileTable()
@brief 构造共享打开文件表，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-22ab25588e11a366"></a>

## `xv::fs::DeviceRegistry::DeviceRegistry`

```doxygen
@fn constexpr xv::fs::DeviceRegistry::DeviceRegistry()
@brief 构造字符设备登记表，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
