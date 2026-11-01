# `kernel/drivers/display/virtio_gpu.cpp`

<a id="fn-3b4ed7f29ddf56dc"></a>

## `xv::drivers::VirtioGpu::packet`

```doxygen
@fn void xv::drivers::VirtioGpu::packet(unsigned offset, const void *data, unsigned bytes)
@brief 按页复制有界控制请求至全部RequestPages，支持高分辨率多页backing条目。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-0b83b7431aa9d7af"></a>

## `xv::drivers::VirtioGpu::negotiate`

```doxygen
@fn bool xv::drivers::VirtioGpu::negotiate()
@brief 只协商modern VERSION_1并要求GPU controlq至少32槽，不改变其它VirtIO设备队列。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-418918c2b76c710d"></a>

## `xv::drivers::VirtioGpu::allocate`

```doxygen
@fn bool xv::drivers::VirtioGpu::allocate()
@brief 逐页申请并清零按配置推导的控制页与显存，发布GPU专用32槽队列。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-bdb7619ca99109e5"></a>

## `xv::drivers::VirtioGpu::submit`

```doxygen
@fn bool xv::drivers::VirtioGpu::submit(unsigned bytes, unsigned response_type, unsigned minimum)
@brief 提交至多16请求页加响应的唯一FENCE链，验证有界完成索引、ID、长度、类型与fence。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-c8cefba6a339c3f6"></a>

## `xv::drivers::VirtioGpu::setup`

```doxygen
@fn bool xv::drivers::VirtioGpu::setup()
@brief 读取首个可用scanout，按启动配置创建2D资源并一次附加完整散页backing。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-b5ed3779f0a0fa3f"></a>

## `xv::drivers::VirtioGpu::present`

```doxygen
@fn bool xv::drivers::VirtioGpu::present(unsigned x, unsigned y, unsigned width, unsigned height, unsigned origin_y)
@brief 以配置stride计算矩形偏移，先transfer后flush并验证每次完成，分别累计两阶段墙钟毫秒。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-da8e12e4f3828f60"></a>

## `xv::drivers::VirtioGpu::offline`

```doxygen
@fn void xv::drivers::VirtioGpu::offline()
@brief 标记离线并重置设备；确认reset完成后才释放DMA，未确认时保留页面防止迟到DMA。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-1e44dc80c26a7ad6"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::found`

```doxygen
@fn static bool xv::drivers::VirtioGpu::InitializeFlow::found(Context &c) noexcept
@brief 流程动作或条件 found；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-9d93ca82e82cdb8d"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::negotiate`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::negotiate(Context &c) noexcept
@brief 只接受modern VirtIO VERSION_1并选择controlq，不启用3D特性。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-db1ece15ecd3bcd9"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::allocate(Context &c) noexcept
@brief 流程动作或条件 allocate；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-8716c88de93dd635"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::setup`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::setup(Context &c) noexcept
@brief 按启动配置查询有效scanout、创建2D资源并附加全部显存散页。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-b12444d82f463210"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioGpu::InitializeFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-7e4cc732ae105cab"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::fail(Context &c) noexcept
@brief 记录错误并返回失败。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-86d94b2d62387fff"></a>

## `xv::drivers::VirtioGpu::InitializeFlow::ready`

```doxygen
@fn static void xv::drivers::VirtioGpu::InitializeFlow::ready(Context &c) noexcept
@brief 流程动作或条件 ready；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-56c766baedcb4f8f"></a>

## `xv::drivers::VirtioGpu::initialize`

```doxygen
@fn void xv::drivers::VirtioGpu::initialize()
@brief 实现 xv::drivers::VirtioGpu::initialize 的模块操作，按声明参数读写其封装状态。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-353b8b6656921f0a"></a>

## `xv::drivers::VirtioGpu::open`

```doxygen
@fn CharacterDevice *xv::drivers::VirtioGpu::open(bool)
@brief 返回 `ready_ ? this:nullptr`。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-c2b8968074a6c023"></a>

## `xv::drivers::VirtioGpu::copy_pixels`

```doxygen
@fn bool xv::drivers::VirtioGpu::copy_pixels(const FramebufferUpdate &update, IoBuffer argument)
@brief 依配置stride及散页边界逐行复制用户像素；失败不提交显示更新。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-f59989396c8306c4"></a>

## `xv::drivers::VirtioGpu::DrawFlow::read`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::read(Context &c) noexcept
@brief 流程动作或条件 read；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-b7cca7da0f02fe49"></a>

## `xv::drivers::VirtioGpu::DrawFlow::valid`

```doxygen
@fn static bool xv::drivers::VirtioGpu::DrawFlow::valid(Context &c) noexcept
@brief 流程动作或条件 valid；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-7fdc7d9e503c8d44"></a>

## `xv::drivers::VirtioGpu::DrawFlow::copy`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::copy(Context &c) noexcept
@brief 逐页复制矩形并累计copy墙钟毫秒；完整复制成功后累计字节量和draw次数，失败不提交。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-76c0057bbd8b8849"></a>

## `xv::drivers::VirtioGpu::DrawFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioGpu::DrawFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-1dd931d231b641d3"></a>

## `xv::drivers::VirtioGpu::DrawFlow::present`

```doxygen
@fn static void xv::drivers::VirtioGpu::DrawFlow::present(Context &c) noexcept
@brief 向host资源传输指定矩形并flush到scanout，两条请求均等待fence完成。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-afdca91566a1ddb9"></a>

## `xv::drivers::VirtioGpu::ioctl`

```doxygen
@fn int xv::drivers::VirtioGpu::ioctl(unsigned request, IoBuffer argument)
@brief 返回帧缓冲尺寸，或以root身份读取完成/分段计时统计及运行矩形绘制图。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-46afcdf81348cbc3"></a>

## `xv::drivers::VirtioGpu::handle_interrupt`

```doxygen
@fn void xv::drivers::VirtioGpu::handle_interrupt()
@brief 确认设备中断、统计IRQ并唤醒完成通道，不在IRQ中取得GPU的SleepLock。
@details 启动分辨率由XV_DISPLAY_WIDTH/HEIGHT配置，默认1280×800，范围640..1920×480..1080；BGRX8888与v1 ABI保持。显存页数和attach请求页数按配置推导，响应页紧随请求；GPU独立32槽环容纳最多16请求页加响应。InitializeFlow/DrawFlow维护生命周期、矩形范围和用户复制；FENCE请求在500ms内完成，reset确认后才释放DMA，失败未确认则保留。运行期通过定时完成通知等待并持sleep锁，启动无当前进程时保留有界轮询；不在spin锁/关中断状态等待。无GPU不分配DMA，不提供运行期模式切换。GET_TIMING使用独立40字节结构累计copy/transfer/flush毫秒、完整复制字节数和draws；GET_STATS保持原32字节ABI。阶段计时含调度和设备等待，不能等同CPU忙碌时间。 GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-1c35df73ab44ab6f"></a>

## `xv::drivers::VirtioGpu::MapFlow::acquire`

```doxygen
@fn static void xv::drivers::VirtioGpu::MapFlow::acquire(Context &c) noexcept
@brief 取得已有映射状态并尝试映射完整像素页集合。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-93fed9e810dd4ceb"></a>

## `xv::drivers::VirtioGpu::MapFlow::acquired`

```doxygen
@fn static bool xv::drivers::VirtioGpu::MapFlow::acquired(Context &c) noexcept
@brief 检查共享设备映射是否成功建立。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-c35ab49b37af58db"></a>

## `xv::drivers::VirtioGpu::MapFlow::publish`

```doxygen
@fn static void xv::drivers::VirtioGpu::MapFlow::publish(Context &c) noexcept
@brief 向调用者复制映射地址、字节数、资源高度和版本。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-08e4034568494479"></a>

## `xv::drivers::VirtioGpu::MapFlow::published`

```doxygen
@fn static bool xv::drivers::VirtioGpu::MapFlow::published(Context &c) noexcept
@brief 检查映射描述是否成功发布。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-11facbe35af003f0"></a>

## `xv::drivers::VirtioGpu::MapFlow::rollback`

```doxygen
@fn static void xv::drivers::VirtioGpu::MapFlow::rollback(Context &c) noexcept
@brief 发布失败时只解除本次新建映射，保留先前已有映射。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-607306c629b9b498"></a>

## `xv::drivers::VirtioGpu::MapFlow::count`

```doxygen
@fn static void xv::drivers::VirtioGpu::MapFlow::count(Context &c) noexcept
@brief 成功首次发布时累计映射次数。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-be8cd6776831ccad"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::read`

```doxygen
@fn static void xv::drivers::VirtioGpu::MappedPresentFlow::read(Context &c) noexcept
@brief 验证当前映射身份并读取有界呈现描述。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-849f99b2443731ff"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::valid`

```doxygen
@fn static bool xv::drivers::VirtioGpu::MappedPresentFlow::valid(Context &c) noexcept
@brief 检查保留字段、可见窗口原点及资源范围内的上传矩形。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-322d80c306a868d9"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::present`

```doxygen
@fn static void xv::drivers::VirtioGpu::MappedPresentFlow::present(Context &c) noexcept
@brief 发布像素写入并执行transfer、窗口选择及flush。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-d194163e8ecf4522"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioGpu::MappedPresentFlow::okay(Context &c) noexcept
@brief 检查呈现命令序列完成结果。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-d8e4a08e317ec9ff"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::commit`

```doxygen
@fn static void xv::drivers::VirtioGpu::MappedPresentFlow::commit(Context &c) noexcept
@brief 累计成功呈现次数并返回0。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

<a id="fn-8efa6714144a9cd5"></a>

## `xv::drivers::VirtioGpu::MappedPresentFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioGpu::MappedPresentFlow::fail(Context &c) noexcept
@brief 呈现失败后沿原设备复位/离线路径保留必要DMA所有权。
@details GPU为两屏高的BGRX8888资源分配散页，32槽独立队列容纳最多16请求页加响应。MAP仅root调用，映射响应复制失败时仅回滚新建映射；旧映射保留。PRESENT校验映射身份、资源边界及origin_y，先完成脏区transfer，再按需修改scanout，最后flush；每步保留fence验证与500ms期限，失败reset确认后释放驱动引用，用户引用保留像素页。原DRAW复制接口兼容并恢复origin=0，旧统计ABI保持。
```

- [`xv::drivers::VirtioGpu::MapFlow::acquire`](virtio_gpu.cpp.md#fn-1c35df73ab44ab6f)

- [`xv::drivers::VirtioGpu::MapFlow::acquired`](virtio_gpu.cpp.md#fn-93fed9e810dd4ceb)

- [`xv::drivers::VirtioGpu::MapFlow::publish`](virtio_gpu.cpp.md#fn-c35ab49b37af58db)

- [`xv::drivers::VirtioGpu::MapFlow::published`](virtio_gpu.cpp.md#fn-08e4034568494479)

- [`xv::drivers::VirtioGpu::MapFlow::rollback`](virtio_gpu.cpp.md#fn-11facbe35af003f0)

- [`xv::drivers::VirtioGpu::MapFlow::count`](virtio_gpu.cpp.md#fn-607306c629b9b498)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::read`](virtio_gpu.cpp.md#fn-be8cd6776831ccad)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::valid`](virtio_gpu.cpp.md#fn-849f99b2443731ff)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::present`](virtio_gpu.cpp.md#fn-322d80c306a868d9)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::okay`](virtio_gpu.cpp.md#fn-d194163e8ecf4522)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::commit`](virtio_gpu.cpp.md#fn-d8e4a08e317ec9ff)

- [`xv::drivers::VirtioGpu::MappedPresentFlow::fail`](virtio_gpu.cpp.md#fn-8efa6714144a9cd5)

- [`xv::drivers::VirtioGpu::allocate`](virtio_gpu.cpp.md#fn-418918c2b76c710d)

- [`xv::drivers::VirtioGpu::copy_pixels`](virtio_gpu.cpp.md#fn-c2b8968074a6c023)

- [`xv::drivers::VirtioGpu::handle_interrupt`](virtio_gpu.cpp.md#fn-46afcdf81348cbc3)

- [`xv::drivers::VirtioGpu::initialize`](virtio_gpu.cpp.md#fn-56c766baedcb4f8f)

- [`xv::drivers::VirtioGpu::ioctl`](virtio_gpu.cpp.md#fn-afdca91566a1ddb9)

- [`xv::drivers::VirtioGpu::negotiate`](virtio_gpu.cpp.md#fn-0b83b7431aa9d7af)

- [`xv::drivers::VirtioGpu::offline`](virtio_gpu.cpp.md#fn-da8e12e4f3828f60)

- [`xv::drivers::VirtioGpu::open`](virtio_gpu.cpp.md#fn-353b8b6656921f0a)

- [`xv::drivers::VirtioGpu::packet`](virtio_gpu.cpp.md#fn-3b4ed7f29ddf56dc)

- [`xv::drivers::VirtioGpu::present`](virtio_gpu.cpp.md#fn-b5ed3779f0a0fa3f)

- [`xv::drivers::VirtioGpu::setup`](virtio_gpu.cpp.md#fn-c8cefba6a339c3f6)

- [`xv::drivers::VirtioGpu::submit`](virtio_gpu.cpp.md#fn-bdb7619ca99109e5)

- [`xv::drivers::VirtioGpu::DrawFlow::copy`](virtio_gpu.cpp.md#fn-7fdc7d9e503c8d44)

- [`xv::drivers::VirtioGpu::DrawFlow::okay`](virtio_gpu.cpp.md#fn-76c0057bbd8b8849)

- [`xv::drivers::VirtioGpu::DrawFlow::present`](virtio_gpu.cpp.md#fn-1dd931d231b641d3)

- [`xv::drivers::VirtioGpu::DrawFlow::read`](virtio_gpu.cpp.md#fn-f59989396c8306c4)

- [`xv::drivers::VirtioGpu::DrawFlow::valid`](virtio_gpu.cpp.md#fn-b7cca7da0f02fe49)

- [`xv::drivers::VirtioGpu::InitializeFlow::allocate`](virtio_gpu.cpp.md#fn-db1ece15ecd3bcd9)

- [`xv::drivers::VirtioGpu::InitializeFlow::fail`](virtio_gpu.cpp.md#fn-7e4cc732ae105cab)

- [`xv::drivers::VirtioGpu::InitializeFlow::found`](virtio_gpu.cpp.md#fn-1e44dc80c26a7ad6)

- [`xv::drivers::VirtioGpu::InitializeFlow::negotiate`](virtio_gpu.cpp.md#fn-9d93ca82e82cdb8d)

- [`xv::drivers::VirtioGpu::InitializeFlow::okay`](virtio_gpu.cpp.md#fn-b12444d82f463210)

- [`xv::drivers::VirtioGpu::InitializeFlow::ready`](virtio_gpu.cpp.md#fn-86d94b2d62387fff)

- [`xv::drivers::VirtioGpu::InitializeFlow::setup`](virtio_gpu.cpp.md#fn-8716c88de93dd635)
