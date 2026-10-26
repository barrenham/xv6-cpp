# `kernel/drivers/tty/pty/pty.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/pty/pty.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:pty`。

直接模块依赖：`:tty`, `:n_tty`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Pty::Endpoint::Endpoint`](pty.cppm.md#fn-2a29792ec8741c5d)
- [`xv::drivers::Pty::Endpoint::close`](pty.cpp.md#fn-2f858e49784ccac1)
- [`xv::drivers::Pty::Endpoint::initialize`](pty.cppm.md#fn-e2b0ec44c77e19ee)
- [`xv::drivers::Pty::Endpoint::ioctl`](pty.cpp.md#fn-6b6a9388f115d902)
- [`xv::drivers::Pty::Endpoint::name`](pty.cppm.md#fn-ee3139078fbb2856)
- [`xv::drivers::Pty::Endpoint::poll`](pty.cpp.md#fn-e30ef808741bf1eb)
- [`xv::drivers::Pty::Endpoint::put_sync`](pty.cppm.md#fn-207df1a0846072b3)
- [`xv::drivers::Pty::Endpoint::read`](pty.cpp.md#fn-2fe0922873329d2b)
- [`xv::drivers::Pty::Endpoint::write`](pty.cpp.md#fn-fce9e782c37be567)
- [`xv::drivers::Pty::Output::Output`](pty.cppm.md#fn-23a70e9b25368462)
- [`xv::drivers::Pty::Output::drain`](pty.cpp.md#fn-f8bb828f54046dc7)
- [`xv::drivers::Pty::Output::put_sync`](pty.cpp.md#fn-c576e0a8d12cb145)
- [`xv::drivers::Pty::Output::transmit`](pty.cpp.md#fn-ee0c89905868fc1c)
- [`xv::drivers::Pty::Output::writable`](pty.cpp.md#fn-b1d57a724fdf31ed)
- [`xv::drivers::Pty::Pty`](pty.cppm.md#fn-641b7bd25da0e490)
- [`xv::drivers::Pty::attach_slave`](pty.cpp.md#fn-71b2427af9997333)
- [`xv::drivers::Pty::close`](pty.cpp.md#fn-5c7a739984ad05b7)
- [`xv::drivers::Pty::initialize`](pty.cpp.md#fn-87ffb0376d0f4542)
- [`xv::drivers::Pty::master`](pty.cppm.md#fn-0467a3889175eb9a)
- [`xv::drivers::Pty::read_master`](pty.cpp.md#fn-26ae6ac8d5631f7d)
- [`xv::drivers::Pty::release`](pty.cpp.md#fn-c67e15beca56bec0)
- [`xv::drivers::Pty::slave`](pty.cppm.md#fn-14b4438a00e7698b)
- [`xv::drivers::Pty::timer_tick`](pty.cppm.md#fn-472fec063d5d3e43)
- [`xv::drivers::Pty::write_master`](pty.cpp.md#fn-f40800f626106299)

<a id="fn-23a70e9b25368462"></a>

## `xv::drivers::Pty::Output::Output`

```doxygen
@fn constexpr explicit xv::drivers::Pty::Output::Output(Pty &pair)
@brief 构造终端输出后端，建立本重载要求的成员初值和借用关系。
@param pair 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2a29792ec8741c5d"></a>

## `xv::drivers::Pty::Endpoint::Endpoint`

```doxygen
@fn constexpr xv::drivers::Pty::Endpoint::Endpoint(Pty &pair, bool master)
@brief 构造PTY 文件端点，建立本重载要求的成员初值和借用关系。
@param pair 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param master 本次操作使用的 主端点 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_` → `master_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ee3139078fbb2856"></a>

## `xv::drivers::Pty::Endpoint::name`

```doxygen
@fn const char *xv::drivers::Pty::Endpoint::name() const override
@brief PTY 文件端点：读取或计算 `master_ ? "pty master" : "pty slave"`，返回当前查询结果。
@return 各分支实际返回：`master_ ? "pty master" : "pty slave"`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e2b0ec44c77e19ee"></a>

## `xv::drivers::Pty::Endpoint::initialize`

```doxygen
@fn void xv::drivers::Pty::Endpoint::initialize() override
@brief PTY 文件端点：建立该对象开始工作所需的初始状态。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-207df1a0846072b3"></a>

## `xv::drivers::Pty::Endpoint::put_sync`

```doxygen
@fn void xv::drivers::Pty::Endpoint::put_sync(int) override
@brief PTY 文件端点：通过不能睡眠的输出路径提交一个字符。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-641b7bd25da0e490"></a>

## `xv::drivers::Pty::Pty`

```doxygen
@fn constexpr xv::drivers::Pty::Pty()
@brief 构造匿名伪终端对，建立本重载要求的成员初值和借用关系。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`output_` → `terminal_` → `master_` → `slave_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-472fec063d5d3e43"></a>

## `xv::drivers::Pty::timer_tick`

```doxygen
@fn void xv::drivers::Pty::timer_tick()
@brief 匿名伪终端对：定时器tick。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`terminal_.timer_tick`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0467a3889175eb9a"></a>

## `xv::drivers::Pty::master`

```doxygen
@fn Endpoint &xv::drivers::Pty::master()
@brief 匿名伪终端对：读取或计算 `master_`，返回当前查询结果。
@return 各分支实际返回：`master_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-14b4438a00e7698b"></a>

## `xv::drivers::Pty::slave`

```doxygen
@fn Endpoint &xv::drivers::Pty::slave()
@brief 匿名伪终端对：读取或计算 `slave_`，返回当前查询结果。
@return 各分支实际返回：`slave_`。应结合上面的条件检查区分正常值、空值和错误值。
```
