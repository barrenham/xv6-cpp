# `kernel/drivers/display/virtio_gpu.cpp`

<a id="fn-3b4ed7f29ddf56dc"></a>

## `xv::drivers::VirtioGpu::packet`

```doxygen
@fn void xv::drivers::VirtioGpu::packet(unsigned offset, const void *data, unsigned bytes)
@brief 按页复制有界控制请求至全部RequestPages，支持高分辨率多页backing条目。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-0b83b7431aa9d7af"></a>

## `xv::drivers::VirtioGpu::negotiate`

```doxygen
@fn bool xv::drivers::VirtioGpu::negotiate()
@brief 只协商modern VERSION_1并要求GPU controlq至少16槽，不改变其它VirtIO设备队列。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-418918c2b76c710d"></a>

## `xv::drivers::VirtioGpu::allocate`

```doxygen
@fn bool xv::drivers::VirtioGpu::allocate()
@brief 逐页申请并清零按配置推导的控制页与显存，发布GPU专用16槽队列。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-bdb7619ca99109e5"></a>

## `xv::drivers::VirtioGpu::submit`

```doxygen
@fn bool xv::drivers::VirtioGpu::submit(unsigned bytes, unsigned response_type, unsigned minimum)
@brief 提交至多8请求页加响应的唯一FENCE链，验证有界完成索引、ID、长度、类型与fence。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-c8cefba6a339c3f6"></a>

## `xv::drivers::VirtioGpu::setup`

```doxygen
@fn bool xv::drivers::VirtioGpu::setup()
@brief 读取首个可用scanout，按启动配置创建2D资源并一次附加完整散页backing。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-b5ed3779f0a0fa3f"></a>

## `xv::drivers::VirtioGpu::present`

```doxygen
@fn bool xv::drivers::VirtioGpu::present(unsigned x, unsigned y, unsigned width, unsigned height)
@brief 以配置stride计算矩形偏移，先transfer后flush并验证每次完成。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-da8e12e4f3828f60"></a>

## `xv::drivers::VirtioGpu::offline`

```doxygen
@fn void xv::drivers::VirtioGpu::offline()
@brief 标记离线并重置设备；确认reset完成后才释放DMA，未确认时保留页面防止迟到DMA。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-1e44dc80c26a7ad6"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::found`

```doxygen
@fn static bool xv::drivers::VirtioGpu::InitializeFlow::found(Context &c) noexcept
@brief 流程动作或条件 found；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-9d93ca82e82cdb8d"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::negotiate`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::negotiate(Context &c) noexcept
@brief 只接受modern VirtIO VERSION_1并选择controlq，不启用3D特性。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-db1ece15ecd3bcd9"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::allocate(Context &c) noexcept
@brief 流程动作或条件 allocate；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-8716c88de93dd635"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::setup`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::setup(Context &c) noexcept
@brief 按启动配置查询有效scanout、创建2D资源并附加全部显存散页。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-b12444d82f463210"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioGpu::InitializeFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-7e4cc732ae105cab"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::fail(Context &c) noexcept
@brief 记录错误并返回失败。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-86d94b2d62387fff"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::ready`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::ready(Context &c) noexcept
@brief 流程动作或条件 ready；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-56c766baedcb4f8f"></a>

## `xv::drivers::VirtioGpu::initialize`

```doxygen
@fn void xv::drivers::VirtioGpu::initialize()
@brief 实现 xv::drivers::VirtioGpu::initialize 的模块操作，按声明参数读写其封装状态。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-353b8b6656921f0a"></a>

## `xv::drivers::VirtioGpu::open`

```doxygen
@fn CharacterDevice *xv::drivers::VirtioGpu::open(bool)
@brief 返回 `ready_ ? this:nullptr`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-c2b8968074a6c023"></a>

## `xv::drivers::VirtioGpu::copy_pixels`

```doxygen
@fn bool xv::drivers::VirtioGpu::copy_pixels(const FramebufferUpdate &update, IoBuffer argument)
@brief 依配置stride及散页边界逐行复制用户像素；失败不提交显示更新。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-f59989396c8306c4"></a>

## `xv::drivers::VirtioGpu::DrawFlow::read`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::read(Context &c) noexcept
@brief 流程动作或条件 read；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-b7cca7da0f02fe49"></a>

## `xv::drivers::VirtioGpu::DrawFlow::valid`

```doxygen
@fn static bool xv::drivers::VirtioGpu::DrawFlow::valid(Context &c) noexcept
@brief 流程动作或条件 valid；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-7fdc7d9e503c8d44"></a>

## `xv::drivers::VirtioGpu::DrawFlow::copy`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::copy(Context &c) noexcept
@brief 将指定快照字段逐页写入sink，失败立即停止。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-76c0057bbd8b8849"></a>

## `xv::drivers::VirtioGpu::DrawFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioGpu::DrawFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-1dd931d231b641d3"></a>

## `xv::drivers::VirtioGpu::DrawFlow::present`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::present(Context &c) noexcept
@brief 向host资源传输指定矩形并flush到scanout，两条请求均等待fence完成。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-afdca91566a1ddb9"></a>

## `xv::drivers::VirtioGpu::ioctl`

```doxygen
@fn int xv::drivers::VirtioGpu::ioctl(unsigned request, IoBuffer argument)
@brief 返回当前配置的帧缓冲尺寸，或以root身份运行矩形校验、复制及显示更新图。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```

<a id="fn-46afcdf81348cbc3"></a>

## `xv::drivers::VirtioGpu::handle_interrupt`

```doxygen
@fn void xv::drivers::VirtioGpu::handle_interrupt()
@brief 实现 xv::drivers::VirtioGpu::handle_interrupt 的模块操作，按声明参数读写其封装状态。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立16槽环容纳最多8请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期持sleep锁，不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。
```
