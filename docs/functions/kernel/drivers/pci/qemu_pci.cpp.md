# `kernel/drivers/pci/qemu_pci.cpp`

## 函数导航

- [`xv::drivers::detail::PciConfigureFlow::bar`](qemu_pci.cpp.md#fn-5433dbf967027bea)
- [`xv::drivers::detail::PciConfigureFlow::disable`](qemu_pci.cpp.md#fn-3da2e41cf0cdd4d6)
- [`xv::drivers::detail::PciConfigureFlow::enable`](qemu_pci.cpp.md#fn-89c9a99b34033071)
- [`xv::drivers::detail::PciConfigureFlow::supported`](qemu_pci.cpp.md#fn-cd8b3a9029dc1fe3)
- [`xv::drivers::detail::configure_e1000`](qemu_pci.cpp.md#fn-6b5e562f0b5ba234)
- [`xv::drivers::detail::find_e1000`](qemu_pci.cpp.md#fn-b3ec24772e304be6)

<a id="fn-b3ec24772e304be6"></a>

## `xv::drivers::detail::find_e1000`

```doxygen
@fn unsigned xv::drivers::detail::find_e1000(unsigned *slots, unsigned capacity)
@brief 扫描 PCI 总线零、function 零的三十二个插槽，按 Intel 8086:100e 标识收集最多 capacity 个设备；不做热插拔或通用枚举。
```

<a id="fn-6b5e562f0b5ba234"></a>

## `xv::drivers::detail::configure_e1000`

```doxygen
@fn bool xv::drivers::detail::configure_e1000(unsigned long registers, unsigned slot)
@brief 核对指定插槽设备标识，暂时关闭内存和总线主控后设置 BAR，再恢复所需命令位；只写命令字段避免清除 PCI 状态。
@details 流程由 PciConfigureFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5433dbf967027bea"></a>

## `xv::drivers::detail::PciConfigureFlow::bar`

```doxygen
@fn static void xv::drivers::detail::PciConfigureFlow::bar(Context &c) noexcept
@brief 按原宽度写入 MMIO BAR。
@details 原入口先拒绝越界插槽；视图由原函数词法作用域持有，验证设备后先禁内存/主控、写 BAR，再恢复命令与 IRQ 能力，最后屏障。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3da2e41cf0cdd4d6"></a>

## `xv::drivers::detail::PciConfigureFlow::disable`

```doxygen
@fn static void xv::drivers::detail::PciConfigureFlow::disable(Context &c) noexcept
@brief 保存低十六位命令并临时禁用内存与总线主控。
@details 原入口先拒绝越界插槽；视图由原函数词法作用域持有，验证设备后先禁内存/主控、写 BAR，再恢复命令与 IRQ 能力，最后屏障。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-89c9a99b34033071"></a>

## `xv::drivers::detail::PciConfigureFlow::enable`

```doxygen
@fn static void xv::drivers::detail::PciConfigureFlow::enable(Context &c) noexcept
@brief 恢复原命令策略并启用内存/主控/IRQ，屏障后记录成功。
@details 原入口先拒绝越界插槽；视图由原函数词法作用域持有，验证设备后先禁内存/主控、写 BAR，再恢复命令与 IRQ 能力，最后屏障。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cd8b3a9029dc1fe3"></a>

## `xv::drivers::detail::PciConfigureFlow::supported`

```doxygen
@fn static bool xv::drivers::detail::PciConfigureFlow::supported(Context &c) noexcept
@brief 读取并验证原 Intel 设备标识。
@details 原入口先拒绝越界插槽；视图由原函数词法作用域持有，验证设备后先禁内存/主控、写 BAR，再恢复命令与 IRQ 能力，最后屏障。 下一节点由图策略声明，业务结果与控制信号分离。
```
