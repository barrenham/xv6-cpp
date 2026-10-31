# `kernel/arch/hardware.cpp`

<a id="fn-19c4b5a6805f3a9f"></a>

## `xv::arch::record_cpu_hardware`

```doxygen
@fn void xv::arch::record_cpu_hardware(unsigned hart)
@brief 在机器态读取当前hart的ISA及实现CSR并保存。
@details 在机器态捕获CPU硬件CSR，online位通过release/acquire发布；proc查询仅返回已上线hart。
```

<a id="fn-f55a5e3eaaf9d02c"></a>

## `xv::arch::mark_cpu_online`

```doxygen
@fn void xv::arch::mark_cpu_online(unsigned hart)
@brief 以release发布当前hart硬件信息已可读取。
@details 在机器态捕获CPU硬件CSR，online位通过release/acquire发布；proc查询仅返回已上线hart。
```

<a id="fn-37618c73149703fe"></a>

## `xv::arch::cpu_hardware`

```doxygen
@fn bool xv::arch::cpu_hardware(unsigned index,CpuHardware &result)
@brief 以acquire确认hart上线后复制硬件信息。
@details 在机器态捕获CPU硬件CSR，online位通过release/acquire发布；proc查询仅返回已上线hart。
```
