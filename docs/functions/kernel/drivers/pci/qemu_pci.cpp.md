# `kernel/drivers/pci/qemu_pci.cpp`

## 函数导航

- [`xv::drivers::detail::configure_e1000`](qemu_pci.cpp.md#fn-6f683a0655461652)


独立 Doxygen 开发文档；运行代码位于 `kernel/`。


<a id="fn-6f683a0655461652"></a>

## `xv::drivers::detail::configure_e1000`

```doxygen
@fn bool xv::drivers::detail::configure_e1000(unsigned long registers)
@brief 在 QEMU virt 的 PCI bus 0、slot 1、function 0 绑定 82540EM。
@param registers BAR0 的板级 MMIO 物理地址。
@return 标识为 8086:100e 且已配置时为 true；未找到时为 false。
@details ECAM 从 0x30000000 访问。编程 BAR 前停用 memory/bus master，再启用两者及 INTx；配置命令寄存器时高 16 位写零，避免误清 PCI status 的 W1C 位。此实现不枚举任意插槽。
```
