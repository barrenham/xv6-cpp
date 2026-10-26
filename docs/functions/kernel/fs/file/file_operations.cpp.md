# `kernel/fs/file/file_operations.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/file_operations.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:operations`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::DeviceOperations::close`](file_operations.cpp.md#fn-a2f8618d17a4bbd9)
- [`xv::fs::DeviceOperations::read`](file_operations.cpp.md#fn-089d182991f30647)
- [`xv::fs::DeviceOperations::stat`](file_operations.cpp.md#fn-c23786ca0a43fccb)
- [`xv::fs::DeviceOperations::write`](file_operations.cpp.md#fn-1d1dfdc42291888e)
- [`xv::fs::InodeOperations::close`](file_operations.cpp.md#fn-79879addfc04da0d)
- [`xv::fs::InodeOperations::read`](file_operations.cpp.md#fn-853914af4b9956ec)
- [`xv::fs::InodeOperations::stat`](file_operations.cpp.md#fn-c7cf1c77367717d2)
- [`xv::fs::InodeOperations::write`](file_operations.cpp.md#fn-1cd777b3ade9b394)
- [`xv::fs::PipeOperations::close`](file_operations.cpp.md#fn-bef638c79d19398f)
- [`xv::fs::PipeOperations::read`](file_operations.cpp.md#fn-8f800cf76ff337a3)
- [`xv::fs::PipeOperations::write`](file_operations.cpp.md#fn-268721c60c117f57)

<a id="fn-79879addfc04da0d"></a>

## `xv::fs::InodeOperations::close`

```doxygen
@fn void xv::fs::InodeOperations::close(File &file)
@brief inode 文件操作适配器：关闭当前端点或释放文件关联。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`filesystem` → `release`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-853914af4b9956ec"></a>

## `xv::fs::InodeOperations::read`

```doxygen
@fn int xv::fs::InodeOperations::read(File &file, unsigned long address, int count)
@brief inode 文件操作适配器：读取已有数据并按接口返回读取结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `filesystem` → `read`。
@details 本函数可见的状态更新包括：`file.offset += result;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`result > 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`result`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1cd777b3ade9b394"></a>

## `xv::fs::InodeOperations::write`

```doxygen
@fn int xv::fs::InodeOperations::write(File &file, unsigned long address, int count)
@brief inode 文件操作适配器：提交输入数据并报告传输结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::killed` → `guard` → `filesystem` → `write`。
@details 本函数可见的状态更新包括：`file.offset += result;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`done < count`；`detail::killed()`；`part > maximum`；`result > 0`；`result != part`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`done ? done : -1`；`done == count ? count : -1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c7cf1c77367717d2"></a>

## `xv::fs::InodeOperations::stat`

```doxygen
@fn int xv::fs::InodeOperations::stat(File &file, unsigned long address)
@brief inode 文件操作适配器：stat。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `filesystem` → `stat` → `detail::copy_out`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`detail::copy_out(true, address, &value, sizeof(value))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a2f8618d17a4bbd9"></a>

## `xv::fs::DeviceOperations::close`

```doxygen
@fn void xv::fs::DeviceOperations::close(File &file)
@brief 字符设备文件操作适配器：关闭当前端点或释放文件关联。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`filesystem` → `release`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-089d182991f30647"></a>

## `xv::fs::DeviceOperations::read`

```doxygen
@fn int xv::fs::DeviceOperations::read(File &file, unsigned long address, int count)
@brief 字符设备文件操作适配器：读取已有数据并按接口返回读取结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`devices` → `get` → `device->read` → `file.nonblocking`。
@details 需要特别跟踪的条件：`!device`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`device->read({xv::drivers::AddressSpace::User, address, file.nonblocking()}, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1d1dfdc42291888e"></a>

## `xv::fs::DeviceOperations::write`

```doxygen
@fn int xv::fs::DeviceOperations::write(File &file, unsigned long address, int count)
@brief 字符设备文件操作适配器：提交输入数据并报告传输结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`devices` → `get` → `device->write` → `file.nonblocking`。
@details 需要特别跟踪的条件：`!device`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`device->write({xv::drivers::AddressSpace::User, address, file.nonblocking()}, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c23786ca0a43fccb"></a>

## `xv::fs::DeviceOperations::stat`

```doxygen
@fn int xv::fs::DeviceOperations::stat(File &file, unsigned long address)
@brief 字符设备文件操作适配器：stat。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `filesystem` → `stat` → `detail::copy_out`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`detail::copy_out(true, address, &value, sizeof(value))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bef638c79d19398f"></a>

## `xv::fs::PipeOperations::close`

```doxygen
@fn void xv::fs::PipeOperations::close(File &file)
@brief 管道文件操作适配器：关闭当前端点或释放文件关联。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file.pipe->close`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8f800cf76ff337a3"></a>

## `xv::fs::PipeOperations::read`

```doxygen
@fn int xv::fs::PipeOperations::read(File &file, unsigned long address, int count)
@brief 管道文件操作适配器：读取已有数据并按接口返回读取结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file.pipe->read` → `file.nonblocking`。
@return 各分支实际返回：`file.pipe->read(address, count, file.nonblocking())`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-268721c60c117f57"></a>

## `xv::fs::PipeOperations::write`

```doxygen
@fn int xv::fs::PipeOperations::write(File &file, unsigned long address, int count)
@brief 管道文件操作适配器：提交输入数据并报告传输结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file.pipe->write` → `file.nonblocking`。
@return 各分支实际返回：`file.pipe->write(address, count, file.nonblocking())`。应结合上面的条件检查区分正常值、空值和错误值。
```
