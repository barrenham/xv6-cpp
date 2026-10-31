# `kernel/drivers/entropy/virtio_entropy.cpp`

<a id="fn-893a4acfcfbfc2e4"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::identify`

```doxygen
@fn static void xv::drivers::VirtioEntropy::InitializeFlow::identify(Context &c) noexcept
@brief 流程动作或条件 identify；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-08fedfea72e2def8"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::found`

```doxygen
@fn static bool xv::drivers::VirtioEntropy::InitializeFlow::found(Context &c) noexcept
@brief 流程动作或条件 found；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-071d742d750eba8c"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::negotiate`

```doxygen
@fn static void xv::drivers::VirtioEntropy::InitializeFlow::negotiate(Context &c) noexcept
@brief 流程动作或条件 negotiate；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-41d1d01f541aea62"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioEntropy::InitializeFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-7477e705b311d7ba"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioEntropy::InitializeFlow::allocate(Context &c) noexcept
@brief 流程动作或条件 allocate；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-7db80345e6c398c7"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioEntropy::InitializeFlow::fail(Context &c) noexcept
@brief 记录错误并返回失败。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-9834014bb83a5377"></a>

## `xv::drivers::VirtioEntropy::InitializeFlow::publish`

```doxygen
@fn static void xv::drivers::VirtioEntropy::InitializeFlow::publish(Context &c) noexcept
@brief 流程动作或条件 publish；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-522ae8ef216974d2"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::available`

```doxygen
@fn static bool xv::drivers::VirtioEntropy::ReadFlow::available(Context &c) noexcept
@brief 流程动作或条件 available；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-cb021fa58c6bc886"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::stopped`

```doxygen
@fn static bool xv::drivers::VirtioEntropy::ReadFlow::stopped(Context &c) noexcept
@brief 流程动作或条件 stopped；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-6e9a16f1df84813d"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::VirtioEntropy::ReadFlow::nonblocking(Context &c) noexcept
@brief 流程动作或条件 nonblocking；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-c598a13938ffca24"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::again`

```doxygen
@fn static void xv::drivers::VirtioEntropy::ReadFlow::again(Context &c) noexcept
@brief 流程动作或条件 again；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-9c537a7662dcb252"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::sleep`

```doxygen
@fn static void xv::drivers::VirtioEntropy::ReadFlow::sleep(Context &c) noexcept
@brief 流程动作或条件 sleep；作用于当前Context，转移关系由所属图声明。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-bd5eb1ab0408c112"></a>

## `xv::drivers::VirtioEntropy::ReadFlow::copy`

```doxygen
@fn static void xv::drivers::VirtioEntropy::ReadFlow::copy(Context &c) noexcept
@brief 将指定快照字段逐页写入sink，失败立即停止。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-9c7aa002244c6a4d"></a>

## `xv::drivers::VirtioEntropy::initialize`

```doxygen
@fn void VirtioEntropy::xv::drivers::VirtioEntropy::initialize()
@brief 实现 xv::drivers::VirtioEntropy::initialize 的模块操作，按声明参数读写其封装状态。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-1b8f5db03dc4706c"></a>

## `xv::drivers::VirtioEntropy::request`

```doxygen
@fn void VirtioEntropy::xv::drivers::VirtioEntropy::request()
@brief 实现 xv::drivers::VirtioEntropy::request 的模块操作，按声明参数读写其封装状态。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-bfbb50f5dd5b89a2"></a>

## `xv::drivers::VirtioEntropy::read`

```doxygen
@fn int VirtioEntropy::xv::drivers::VirtioEntropy::read(IoBuffer destination,int count)
@brief 实现 xv::drivers::VirtioEntropy::read 的模块操作，按声明参数读写其封装状态。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-401f1b4055460578"></a>

## `xv::drivers::VirtioEntropy::poll`

```doxygen
@fn unsigned VirtioEntropy::xv::drivers::VirtioEntropy::poll(xv::sync::PollTable *table)
@brief 返回 `!ready_ ? POLLERR:read_<size_ ? POLLIN:0`。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-8613f104753364cd"></a>

## `xv::drivers::VirtioEntropy::handle_interrupt`

```doxygen
@fn void VirtioEntropy::xv::drivers::VirtioEntropy::handle_interrupt()
@brief 实现 xv::drivers::VirtioEntropy::handle_interrupt 的模块操作，按声明参数读写其封装状态。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```
