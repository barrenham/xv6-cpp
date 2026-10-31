# `kernel/drivers/tty/core/tty.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/core/tty.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:tty`。

直接模块依赖：`:character`, `:ldisc`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Tty::hangup`](tty.cpp.md#fn-3acc0f0e4a2096c9)
- [`xv::drivers::Tty::initialize`](tty.cpp.md#fn-2a48d8f477ebb1b7)
- [`xv::drivers::Tty::inject`](tty.cpp.md#fn-258d0222cee0993a)
- [`xv::drivers::Tty::input_writable`](tty.cpp.md#fn-ec3f40ec3ea02161)
- [`xv::drivers::Tty::ioctl`](tty.cpp.md#fn-3625e17fbe0b06ab)
- [`xv::drivers::Tty::name`](tty.cppm.md#fn-bd71d07141df44bc)
- [`xv::drivers::Tty::poll`](tty.cpp.md#fn-f52d9721bfdab4d5)
- [`xv::drivers::Tty::put_sync`](tty.cpp.md#fn-d96051693f210b8e)
- [`xv::drivers::Tty::read`](tty.cpp.md#fn-2b943588ba88bb06)
- [`xv::drivers::Tty::receive`](tty.cpp.md#fn-a871a5e9e5abafda)
- [`xv::drivers::Tty::timer_tick`](tty.cpp.md#fn-eaea2170608ca21d)
- [`xv::drivers::Tty::write`](tty.cpp.md#fn-7a0276d465b23c53)

<a id="fn-bd71d07141df44bc"></a>

## `xv::drivers::Tty::name`

```doxygen
@fn const char *xv::drivers::Tty::name() const override
@brief 终端核心：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-536b8598df7afbe4"></a>

## `xv::drivers::TerminalLifetime::retain_terminal`

```doxygen
@fn virtual void xv::drivers::TerminalLifetime::retain_terminal()=0
@brief 原子增加终端对象寿命引用，不获取PTY锁。
@details 终端寿命先在进程组锁内原子固定，再在该锁外取得PTY锁完成打开；关闭释放对应引用。
```

<a id="fn-62b153502cd3a57e"></a>

## `xv::drivers::TerminalLifetime::open_retained_terminal`

```doxygen
@fn virtual CharacterDevice *xv::drivers::TerminalLifetime::open_retained_terminal()=0
@brief 消费已固定的终端引用，在group锁外完成slave登记和打开。
@details 终端寿命先在进程组锁内原子固定，再在该锁外取得PTY锁完成打开；关闭释放对应引用。
```

<a id="fn-d5d21a61ca1b6b3b"></a>

## `xv::drivers::TerminalLifetime::TerminalLifetime`

```doxygen
@fn constexpr xv::drivers::TerminalLifetime::TerminalLifetime()=default
@brief 构造对象并建立成员初值及借用关系。
@details 终端寿命先在进程组锁内原子固定，再在该锁外取得PTY锁完成打开；关闭释放对应引用。
```

<a id="fn-cb0619389216bc3b"></a>

## `xv::drivers::TerminalLifetime::~TerminalLifetime`

```doxygen
@fn xv::drivers::TerminalLifetime::~TerminalLifetime()=default
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details 终端寿命先在进程组锁内原子固定，再在该锁外取得PTY锁完成打开；关闭释放对应引用。
```

<a id="fn-a9dea36568afe117"></a>

## `xv::drivers::Tty::Tty`

```doxygen
@fn constexpr xv::drivers::Tty::Tty(TerminalOutput &output, LineDiscipline &discipline,TerminalLifetime *lifetime=nullptr) : output_(output), discipline_(discipline), lifetime_(lifetime)
@brief 构造终端核心，建立本重载要求的成员初值和借用关系。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-3132483920f71056"></a>

## `xv::drivers::Tty::pin_terminal`

```doxygen
@fn Tty *xv::drivers::Tty::pin_terminal()
@brief 在group锁内固定控制终端的寿命，完成打开必须在锁外。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-a47f5e5b4533d508"></a>

## `xv::drivers::Tty::open_pinned_terminal`

```doxygen
@fn CharacterDevice *xv::drivers::Tty::open_pinned_terminal()
@brief 返回 `lifetime_ ? lifetime_->open_retained_terminal():this`。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
