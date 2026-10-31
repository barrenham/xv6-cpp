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
