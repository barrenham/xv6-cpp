# `kernel/drivers/pci/qemu_pci.cpp`

## 函数导航

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
```
