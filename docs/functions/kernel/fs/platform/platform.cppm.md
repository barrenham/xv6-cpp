# `kernel/fs/platform/platform.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/platform/platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:platform`。

直接模块依赖：`:inode`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::detail::allocate_page`](kernel_platform.cpp.md#fn-ac353f7455a3e838)
- [`xv::fs::detail::compare`](kernel_platform.cpp.md#fn-2157cfabe1773e44)
- [`xv::fs::detail::copy_fixed`](kernel_platform.cpp.md#fn-44100aabfc8b85fc)
- [`xv::fs::detail::copy_in`](kernel_platform.cpp.md#fn-73ff94af3db47c5c)
- [`xv::fs::detail::copy_out`](kernel_platform.cpp.md#fn-092f21133e90643a)
- [`xv::fs::detail::current_directory`](kernel_platform.cpp.md#fn-f7b6f6a96cd5d89c)
- [`xv::fs::detail::fail`](kernel_platform.cpp.md#fn-15b1609ad1be02bf)
- [`xv::fs::detail::fill`](kernel_platform.cpp.md#fn-1f96e9ad94b54ca8)
- [`xv::fs::detail::free_page`](kernel_platform.cpp.md#fn-2edb4be43f545533)
- [`xv::fs::detail::killed`](kernel_platform.cpp.md#fn-a4b206516ef7c6e3)
- [`xv::fs::detail::move`](kernel_platform.cpp.md#fn-7edb69559eaac2a2)
- [`xv::fs::detail::prepare_interruptible_sleep`](kernel_platform.cpp.md#fn-fcf4728d931b520f)
- [`xv::fs::detail::prepare_sleep`](kernel_platform.cpp.md#fn-ab640082c3d8f293)
- [`xv::fs::detail::report`](kernel_platform.cpp.md#fn-f79daff3187ea060)
- [`xv::fs::detail::sleep`](kernel_platform.cpp.md#fn-cc457fade9ae62c6)
- [`xv::fs::detail::wakeup`](kernel_platform.cpp.md#fn-aec05456dbae85c7)

