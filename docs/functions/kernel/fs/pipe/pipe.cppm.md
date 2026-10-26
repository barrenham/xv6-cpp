# `kernel/fs/pipe/pipe.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/pipe/pipe.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:pipe`。

直接模块依赖：`:file`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Pipe::Pipe`](pipe.cppm.md#fn-17230c11ddb2dd73)
- [`xv::fs::Pipe::close`](pipe.cpp.md#fn-bc84a084f8763d9f)
- [`xv::fs::Pipe::initialize`](pipe.cpp.md#fn-506325f4cc7a1e4d)
- [`xv::fs::Pipe::poll`](pipe.cpp.md#fn-394c4a6f8b4d5c75)
- [`xv::fs::Pipe::read`](pipe.cpp.md#fn-64b695cc57f76b14)
- [`xv::fs::Pipe::write`](pipe.cpp.md#fn-dd4843c53ed371db)

<a id="fn-17230c11ddb2dd73"></a>

## `xv::fs::Pipe::Pipe`

```doxygen
@fn constexpr xv::fs::Pipe::Pipe()
@brief 构造有界管道环形缓冲，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
