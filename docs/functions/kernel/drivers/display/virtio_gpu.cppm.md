# `kernel/drivers/display/virtio_gpu.cppm`

<a id="fn-368b4ac766cd532a"></a>

## `xv::drivers::VirtioGpu::VirtioGpu`

```doxygen
@fn constexpr xv::drivers::VirtioGpu::VirtioGpu(unsigned long base, unsigned irq) : registers_(base), irq_(irq)
@brief 构造对象并建立成员初值及借用关系。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-b370cbf85f0ac4f9"></a>

## `xv::drivers::VirtioGpu::name`

```doxygen
@fn const char *xv::drivers::VirtioGpu::name() const override
@brief 返回 `"VirtIO GPU 2D"`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-d4b08fb2a8940104"></a>

## `xv::drivers::VirtioGpu::irq`

```doxygen
@fn unsigned xv::drivers::VirtioGpu::irq() const override
@brief 返回 `irq_`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-fbfce7ba5c527106"></a>

## `xv::drivers::VirtioGpu::read`

```doxygen
@fn int xv::drivers::VirtioGpu::read(IoBuffer, int count) override
@brief 返回 `count==0 ? 0:-1`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-04ab776dc81828f6"></a>

## `xv::drivers::VirtioGpu::write`

```doxygen
@fn int xv::drivers::VirtioGpu::write(IoBuffer, int count) override
@brief 返回 `count==0 ? 0:-1`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-dea5e01690011a18"></a>

## `xv::drivers::VirtioGpu::put_sync`

```doxygen
@fn void xv::drivers::VirtioGpu::put_sync(int) override
@brief 实现 xv::drivers::VirtioGpu::put_sync 的模块操作，按声明参数读写其封装状态。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

- [`xv::drivers::VirtioGpu::VirtioGpu`](virtio_gpu.cppm.md#fn-368b4ac766cd532a)

- [`xv::drivers::VirtioGpu::allocate`](virtio_gpu.cpp.md#fn-418918c2b76c710d)

- [`xv::drivers::VirtioGpu::copy_pixels`](virtio_gpu.cpp.md#fn-c2b8968074a6c023)

- [`xv::drivers::VirtioGpu::handle_interrupt`](virtio_gpu.cpp.md#fn-46afcdf81348cbc3)

- [`xv::drivers::VirtioGpu::initialize`](virtio_gpu.cpp.md#fn-56c766baedcb4f8f)

- [`xv::drivers::VirtioGpu::ioctl`](virtio_gpu.cpp.md#fn-afdca91566a1ddb9)

- [`xv::drivers::VirtioGpu::irq`](virtio_gpu.cppm.md#fn-d4b08fb2a8940104)

- [`xv::drivers::VirtioGpu::name`](virtio_gpu.cppm.md#fn-b370cbf85f0ac4f9)

- [`xv::drivers::VirtioGpu::negotiate`](virtio_gpu.cpp.md#fn-0b83b7431aa9d7af)

- [`xv::drivers::VirtioGpu::offline`](virtio_gpu.cpp.md#fn-da8e12e4f3828f60)

- [`xv::drivers::VirtioGpu::open`](virtio_gpu.cpp.md#fn-353b8b6656921f0a)

- [`xv::drivers::VirtioGpu::packet`](virtio_gpu.cpp.md#fn-3b4ed7f29ddf56dc)

- [`xv::drivers::VirtioGpu::present`](virtio_gpu.cpp.md#fn-b5ed3779f0a0fa3f)

- [`xv::drivers::VirtioGpu::put_sync`](virtio_gpu.cppm.md#fn-dea5e01690011a18)

- [`xv::drivers::VirtioGpu::read`](virtio_gpu.cppm.md#fn-fbfce7ba5c527106)

- [`xv::drivers::VirtioGpu::setup`](virtio_gpu.cpp.md#fn-c8cefba6a339c3f6)

- [`xv::drivers::VirtioGpu::submit`](virtio_gpu.cpp.md#fn-bdb7619ca99109e5)

- [`xv::drivers::VirtioGpu::write`](virtio_gpu.cppm.md#fn-04ab776dc81828f6)
