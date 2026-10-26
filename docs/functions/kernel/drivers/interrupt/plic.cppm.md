# `kernel/drivers/interrupt/plic.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/plic.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:plic`。

直接模块依赖：`:interrupt`, `:mmio`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Plic::Plic`](plic.cppm.md#fn-63df6baf83ed1dd5)
- [`xv::drivers::Plic::claim`](plic.cpp.md#fn-2f59f51d7a3e8b75)
- [`xv::drivers::Plic::complete`](plic.cpp.md#fn-35f1dc5c24401299)
- [`xv::drivers::Plic::enable`](plic.cpp.md#fn-dde1bb3588499d54)
- [`xv::drivers::Plic::initialize`](plic.cpp.md#fn-8371618adfdd12ad)
- [`xv::drivers::Plic::initialize_hart`](plic.cpp.md#fn-ad5f053e65d2ed98)
- [`xv::drivers::Plic::name`](plic.cppm.md#fn-1a30e68c427a8ebb)
- [`xv::drivers::Plic::set_priority`](plic.cpp.md#fn-cdb74cfb3aaaa599)

<a id="fn-63df6baf83ed1dd5"></a>

## `xv::drivers::Plic::Plic`

```doxygen
@fn constexpr explicit xv::drivers::Plic::Plic(unsigned long base)
@brief 构造PLIC 外部中断控制器，建立本重载要求的成员初值和借用关系。
@param base 本次操作使用的 base 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1a30e68c427a8ebb"></a>

## `xv::drivers::Plic::name`

```doxygen
@fn const char *xv::drivers::Plic::name() const override
@brief PLIC 外部中断控制器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
