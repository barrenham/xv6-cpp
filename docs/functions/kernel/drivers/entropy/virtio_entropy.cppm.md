# `kernel/drivers/entropy/virtio_entropy.cppm`

<a id="fn-3af457fc3762ef48"></a>

## `xv::drivers::VirtioEntropy::VirtioEntropy`

```doxygen
@fn constexpr xv::drivers::VirtioEntropy::VirtioEntropy(unsigned long base,unsigned irq):registers_(base),irq_(irq)
@brief 构造对象并建立成员初值及借用关系。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-00e1d1f566beb2db"></a>

## `xv::drivers::VirtioEntropy::name`

```doxygen
@fn const char *xv::drivers::VirtioEntropy::name() const override
@brief 返回 `"VirtIO entropy"`。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-06c9e3fd76dd216c"></a>

## `xv::drivers::VirtioEntropy::irq`

```doxygen
@fn unsigned xv::drivers::VirtioEntropy::irq() const override
@brief 返回 `irq_`。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-aa63e33336ec1add"></a>

## `xv::drivers::VirtioEntropy::write`

```doxygen
@fn int xv::drivers::VirtioEntropy::write(IoBuffer,int count) override
@brief 返回 `count==0 ? 0:-1`。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

<a id="fn-0aad82b9ade0ecd3"></a>

## `xv::drivers::VirtioEntropy::put_sync`

```doxygen
@fn void xv::drivers::VirtioEntropy::put_sync(int) override
@brief 实现 xv::drivers::VirtioEntropy::put_sync 的模块操作，按声明参数读写其封装状态。
@details VirtIO熵设备；一个descriptor在途，DMA有效数据最多4096字节，一次read最多256字节。设备不可用不伪造随机数据；nonblock无数据返回-EAGAIN，用户复制失败不消费。
```

- [`xv::drivers::VirtioEntropy::VirtioEntropy`](virtio_entropy.cppm.md#fn-3af457fc3762ef48)

- [`xv::drivers::VirtioEntropy::handle_interrupt`](virtio_entropy.cpp.md#fn-8613f104753364cd)

- [`xv::drivers::VirtioEntropy::initialize`](virtio_entropy.cpp.md#fn-9c7aa002244c6a4d)

- [`xv::drivers::VirtioEntropy::irq`](virtio_entropy.cppm.md#fn-06c9e3fd76dd216c)

- [`xv::drivers::VirtioEntropy::name`](virtio_entropy.cppm.md#fn-00e1d1f566beb2db)

- [`xv::drivers::VirtioEntropy::poll`](virtio_entropy.cpp.md#fn-401f1b4055460578)

- [`xv::drivers::VirtioEntropy::put_sync`](virtio_entropy.cppm.md#fn-0aad82b9ade0ecd3)

- [`xv::drivers::VirtioEntropy::read`](virtio_entropy.cpp.md#fn-bfbb50f5dd5b89a2)

- [`xv::drivers::VirtioEntropy::request`](virtio_entropy.cpp.md#fn-1b8f5db03dc4706c)

- [`xv::drivers::VirtioEntropy::write`](virtio_entropy.cppm.md#fn-aa63e33336ec1add)
