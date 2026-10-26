# `kernel/arch/riscv/register_operations.inc`

开发职责：RISC-V 特权寄存器和内存/指令可见性。

源码：[打开实际文件](../../../../../kernel/arch/riscv/register_operations.inc)。文档不会加入源码或生成额外 C++ 声明。

## 函数导航

- [`c_sstatus`](register_operations.inc.md#fn-69e390038b961db8)
- [`icache_fence`](register_operations.inc.md#fn-1ece543427fe6d3a)
- [`intr_get`](register_operations.inc.md#fn-7be3c895ea01cfe4)
- [`intr_off`](register_operations.inc.md#fn-b997376eda716a25)
- [`intr_on`](register_operations.inc.md#fn-e76aacbeb004ccd6)
- [`io_fence`](register_operations.inc.md#fn-76cd5dd1fd2ffcb5)
- [`r_fp`](register_operations.inc.md#fn-0d482401e77b7653)
- [`r_mcounteren`](register_operations.inc.md#fn-1093ea092751cbc1)
- [`r_medeleg`](register_operations.inc.md#fn-98313d7258820389)
- [`r_menvcfg`](register_operations.inc.md#fn-eed8bcb7e9f1fcab)
- [`r_mhartid`](register_operations.inc.md#fn-e0b3a7416232f988)
- [`r_mideleg`](register_operations.inc.md#fn-b3123c7a83bdf417)
- [`r_mie`](register_operations.inc.md#fn-df557fda6783760f)
- [`r_mstatus`](register_operations.inc.md#fn-a10503a6316bf5e3)
- [`r_mtvec`](register_operations.inc.md#fn-bdc7065b910cc99a)
- [`r_ra`](register_operations.inc.md#fn-1a59827948a9473c)
- [`r_satp`](register_operations.inc.md#fn-bb9149946c5fd993)
- [`r_scause`](register_operations.inc.md#fn-2deee52ed5e58668)
- [`r_sepc`](register_operations.inc.md#fn-3de8984db776c3ed)
- [`r_sie`](register_operations.inc.md#fn-c3cc7215f84d621b)
- [`r_sip`](register_operations.inc.md#fn-38c54174efabd2e4)
- [`r_sp`](register_operations.inc.md#fn-be08f0a77733302d)
- [`r_sstatus`](register_operations.inc.md#fn-dad5d953f0208139)
- [`r_stimecmp`](register_operations.inc.md#fn-c000f02fd8a97f6d)
- [`r_stval`](register_operations.inc.md#fn-2c39fd2fefb99be2)
- [`r_stvec`](register_operations.inc.md#fn-f21cf19ea52046fd)
- [`r_time`](register_operations.inc.md#fn-c28dc4f0e173c489)
- [`r_tp`](register_operations.inc.md#fn-4cd8890830f0c543)
- [`rc_sstatus`](register_operations.inc.md#fn-630fc69b3e6ad197)
- [`s_sstatus`](register_operations.inc.md#fn-2494c12a1ab635f2)
- [`sfence_vma`](register_operations.inc.md#fn-3d44b2702e7c5e11)
- [`w_mcounteren`](register_operations.inc.md#fn-08799499a5c0917a)
- [`w_medeleg`](register_operations.inc.md#fn-707489b940e2567e)
- [`w_menvcfg`](register_operations.inc.md#fn-f5ccea983416da7c)
- [`w_mepc`](register_operations.inc.md#fn-202518556b1726ac)
- [`w_mideleg`](register_operations.inc.md#fn-7338fd9a60652d3f)
- [`w_mie`](register_operations.inc.md#fn-3aa50455e63d6d10)
- [`w_mstatus`](register_operations.inc.md#fn-4faf7ff0db40fadb)
- [`w_mtvec`](register_operations.inc.md#fn-2390624f2f47d445)
- [`w_pmpaddr0`](register_operations.inc.md#fn-bda1fddf68758477)
- [`w_pmpcfg0`](register_operations.inc.md#fn-99dded37f5800f87)
- [`w_satp`](register_operations.inc.md#fn-4091f9840161beb2)
- [`w_sepc`](register_operations.inc.md#fn-a808715a5b3e7e89)
- [`w_sie`](register_operations.inc.md#fn-118cf86c976736b2)
- [`w_sip`](register_operations.inc.md#fn-8345de1fb2990a08)
- [`w_sstatus`](register_operations.inc.md#fn-277a2c9c47353026)
- [`w_stimecmp`](register_operations.inc.md#fn-5588f93ac0dfbe22)
- [`w_stvec`](register_operations.inc.md#fn-8e2600bd1d37c465)
- [`w_tp`](register_operations.inc.md#fn-6f10e0ea8bc3524c)

<a id="fn-e0b3a7416232f988"></a>

## `r_mhartid`

```doxygen
@fn inline uint64 r_mhartid()
@brief 读取 RISC-V mhartid 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-a10503a6316bf5e3"></a>

## `r_mstatus`

```doxygen
@fn inline uint64 r_mstatus()
@brief 读取 RISC-V mstatus 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-4faf7ff0db40fadb"></a>

## `w_mstatus`

```doxygen
@fn inline void w_mstatus(uint64 x)
@brief 写入 RISC-V mstatus 寄存器。
@param x 写入 mstatus 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-202518556b1726ac"></a>

## `w_mepc`

```doxygen
@fn inline void w_mepc(uint64 x)
@brief 写入 RISC-V mepc 寄存器。
@param x 写入 mepc 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-dad5d953f0208139"></a>

## `r_sstatus`

```doxygen
@fn inline uint64 r_sstatus()
@brief 读取 RISC-V sstatus 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-277a2c9c47353026"></a>

## `w_sstatus`

```doxygen
@fn inline void w_sstatus(uint64 x)
@brief 写入 RISC-V sstatus 寄存器。
@param x 写入 sstatus 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-2494c12a1ab635f2"></a>

## `s_sstatus`

```doxygen
@fn inline void s_sstatus(uint64 x)
@brief 设置选定位 RISC-V sstatus 寄存器。
@param x 要设置或清除的 sstatus 位掩码。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__volatile__`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-69e390038b961db8"></a>

## `c_sstatus`

```doxygen
@fn inline void c_sstatus(uint64 x)
@brief 清除选定位 RISC-V sstatus 寄存器。
@param x 要设置或清除的 sstatus 位掩码。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__volatile__`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-630fc69b3e6ad197"></a>

## `rc_sstatus`

```doxygen
@fn inline uint64 rc_sstatus(uint64 x)
@brief 读取并清除选定位 RISC-V sstatus 寄存器。
@param x 要设置或清除的 sstatus 位掩码。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__volatile__`。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-38c54174efabd2e4"></a>

## `r_sip`

```doxygen
@fn inline uint64 r_sip()
@brief 读取 RISC-V sip 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-8345de1fb2990a08"></a>

## `w_sip`

```doxygen
@fn inline void w_sip(uint64 x)
@brief 写入 RISC-V sip 寄存器。
@param x 写入 sip 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-c3cc7215f84d621b"></a>

## `r_sie`

```doxygen
@fn inline uint64 r_sie()
@brief 读取 RISC-V sie 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-118cf86c976736b2"></a>

## `w_sie`

```doxygen
@fn inline void w_sie(uint64 x)
@brief 写入 RISC-V sie 寄存器。
@param x 写入 sie 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-df557fda6783760f"></a>

## `r_mie`

```doxygen
@fn inline uint64 r_mie()
@brief 读取 RISC-V mie 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-3aa50455e63d6d10"></a>

## `w_mie`

```doxygen
@fn inline void w_mie(uint64 x)
@brief 写入 RISC-V mie 寄存器。
@param x 写入 mie 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-a808715a5b3e7e89"></a>

## `w_sepc`

```doxygen
@fn inline void w_sepc(uint64 x)
@brief 写入 RISC-V sepc 寄存器。
@param x 写入 sepc 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-3de8984db776c3ed"></a>

## `r_sepc`

```doxygen
@fn inline uint64 r_sepc()
@brief 读取 RISC-V sepc 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-98313d7258820389"></a>

## `r_medeleg`

```doxygen
@fn inline uint64 r_medeleg()
@brief 读取 RISC-V medeleg 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-707489b940e2567e"></a>

## `w_medeleg`

```doxygen
@fn inline void w_medeleg(uint64 x)
@brief 写入 RISC-V medeleg 寄存器。
@param x 写入 medeleg 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-b3123c7a83bdf417"></a>

## `r_mideleg`

```doxygen
@fn inline uint64 r_mideleg()
@brief 读取 RISC-V mideleg 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-7338fd9a60652d3f"></a>

## `w_mideleg`

```doxygen
@fn inline void w_mideleg(uint64 x)
@brief 写入 RISC-V mideleg 寄存器。
@param x 写入 mideleg 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-8e2600bd1d37c465"></a>

## `w_stvec`

```doxygen
@fn inline void w_stvec(uint64 x)
@brief 写入 RISC-V stvec 寄存器。
@param x 写入 stvec 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-f21cf19ea52046fd"></a>

## `r_stvec`

```doxygen
@fn inline uint64 r_stvec()
@brief 读取 RISC-V stvec 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-2390624f2f47d445"></a>

## `w_mtvec`

```doxygen
@fn inline void w_mtvec(uint64 x)
@brief 写入 RISC-V mtvec 寄存器。
@param x 写入 mtvec 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-bdc7065b910cc99a"></a>

## `r_mtvec`

```doxygen
@fn inline uint64 r_mtvec()
@brief 读取 RISC-V mtvec 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-c000f02fd8a97f6d"></a>

## `r_stimecmp`

```doxygen
@fn inline uint64 r_stimecmp()
@brief 读取 RISC-V stimecmp 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-5588f93ac0dfbe22"></a>

## `w_stimecmp`

```doxygen
@fn inline void w_stimecmp(uint64 x)
@brief 写入 RISC-V stimecmp 寄存器。
@param x 写入 stimecmp 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-eed8bcb7e9f1fcab"></a>

## `r_menvcfg`

```doxygen
@fn inline uint64 r_menvcfg()
@brief 读取 RISC-V menvcfg 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-f5ccea983416da7c"></a>

## `w_menvcfg`

```doxygen
@fn inline void w_menvcfg(uint64 x)
@brief 写入 RISC-V menvcfg 寄存器。
@param x 写入 menvcfg 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-99dded37f5800f87"></a>

## `w_pmpcfg0`

```doxygen
@fn inline void w_pmpcfg0(uint64 x)
@brief 写入 RISC-V pmpcfg0 寄存器。
@param x 写入 pmpcfg0 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-bda1fddf68758477"></a>

## `w_pmpaddr0`

```doxygen
@fn inline void w_pmpaddr0(uint64 x)
@brief 写入 RISC-V pmpaddr0 寄存器。
@param x 写入 pmpaddr0 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-4091f9840161beb2"></a>

## `w_satp`

```doxygen
@fn inline void w_satp(uint64 x)
@brief 写入 RISC-V satp 寄存器。
@param x 写入 satp 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。 修改页表根还必须配合正确的地址翻译屏障；写入 token 本身不等于回收旧页表安全。
```

<a id="fn-bb9149946c5fd993"></a>

## `r_satp`

```doxygen
@fn inline uint64 r_satp()
@brief 读取 RISC-V satp 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。 修改页表根还必须配合正确的地址翻译屏障；写入 token 本身不等于回收旧页表安全。
```

<a id="fn-2deee52ed5e58668"></a>

## `r_scause`

```doxygen
@fn inline uint64 r_scause()
@brief 读取 RISC-V scause 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-2c39fd2fefb99be2"></a>

## `r_stval`

```doxygen
@fn inline uint64 r_stval()
@brief 读取 RISC-V stval 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 supervisor 级控制/状态寄存器访问，调用上下文必须具备相应权限。读写本 hart 状态不自动同步其他 hart，也不自动为普通内存建立完整可见性协议。
```

<a id="fn-08799499a5c0917a"></a>

## `w_mcounteren`

```doxygen
@fn inline void w_mcounteren(uint64 x)
@brief 写入 RISC-V mcounteren 寄存器。
@param x 写入 mcounteren 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-1093ea092751cbc1"></a>

## `r_mcounteren`

```doxygen
@fn inline uint64 r_mcounteren()
@brief 读取 RISC-V mcounteren 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这是 M 模式控制/状态寄存器访问，应沿 boot::enter 的启动权限路径使用；不能从普通用户态或任意 S 模式代码调用。
```

<a id="fn-c28dc4f0e173c489"></a>

## `r_time`

```doxygen
@fn inline uint64 r_time()
@brief 读取 RISC-V time 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 读取硬件时间计数，不是全局 ticks。用户态是否可读受计数器权限配置影响；等待毫秒与 tick 的换算由上层时钟实现决定。
```

<a id="fn-e76aacbeb004ccd6"></a>

## `intr_on`

```doxygen
@fn inline void intr_on()
@brief RISC-V 特权寄存器和内存/指令可见性：intr进入。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`s_sstatus`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 只操作或查询当前 hart 的 supervisor 中断使能。需要可嵌套的临界区时应使用同步层的 push/pop 协议，不能用直接 intr_on 覆盖外层原本关闭的状态。
```

<a id="fn-b997376eda716a25"></a>

## `intr_off`

```doxygen
@fn inline void intr_off()
@brief RISC-V 特权寄存器和内存/指令可见性：intr关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`c_sstatus`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 只操作或查询当前 hart 的 supervisor 中断使能。需要可嵌套的临界区时应使用同步层的 push/pop 协议，不能用直接 intr_on 覆盖外层原本关闭的状态。
```

<a id="fn-7be3c895ea01cfe4"></a>

## `intr_get`

```doxygen
@fn inline int intr_get()
@brief RISC-V 特权寄存器和内存/指令可见性：读取或计算 `(x & SSTATUS_SIE) != 0`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`r_sstatus`。
@return 各分支实际返回：`(x & SSTATUS_SIE) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 只操作或查询当前 hart 的 supervisor 中断使能。需要可嵌套的临界区时应使用同步层的 push/pop 协议，不能用直接 intr_on 覆盖外层原本关闭的状态。
```

<a id="fn-be08f0a77733302d"></a>

## `r_sp`

```doxygen
@fn inline uint64 r_sp()
@brief 读取 RISC-V sp 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这里访问通用寄存器，不是特权 CSR。内核把 tp 用作 hart 身份；调度迁移、trap 恢复及用户线程指针不能混为同一个生命周期。
```

<a id="fn-0d482401e77b7653"></a>

## `r_fp`

```doxygen
@fn inline uint64 r_fp()
@brief 读取 RISC-V fp 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这里访问通用寄存器，不是特权 CSR。内核把 tp 用作 hart 身份；调度迁移、trap 恢复及用户线程指针不能混为同一个生命周期。
```

<a id="fn-4cd8890830f0c543"></a>

## `r_tp`

```doxygen
@fn inline uint64 r_tp()
@brief 读取 RISC-V tp 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这里访问通用寄存器，不是特权 CSR。内核把 tp 用作 hart 身份；调度迁移、trap 恢复及用户线程指针不能混为同一个生命周期。
```

<a id="fn-6f10e0ea8bc3524c"></a>

## `w_tp`

```doxygen
@fn inline void w_tp(uint64 x)
@brief 写入 RISC-V tp 寄存器。
@param x 写入 tp 的寄存器编码值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这里访问通用寄存器，不是特权 CSR。内核把 tp 用作 hart 身份；调度迁移、trap 恢复及用户线程指针不能混为同一个生命周期。
```

<a id="fn-1a59827948a9473c"></a>

## `r_ra`

```doxygen
@fn inline uint64 r_ra()
@brief 读取 RISC-V ra 寄存器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`x`。应结合上面的条件检查区分正常值、空值和错误值。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 这里访问通用寄存器，不是特权 CSR。内核把 tp 用作 hart 身份；调度迁移、trap 恢复及用户线程指针不能混为同一个生命周期。
```

<a id="fn-3d44b2702e7c5e11"></a>

## `sfence_vma`

```doxygen
@fn inline void sfence_vma()
@brief RISC-V 特权寄存器和内存/指令可见性：sfencevma。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 此处按汇编操作数刷新当前 hart 的地址翻译缓存；它不是跨 hart 的 TLB shootdown，也不会替调用方管理旧映射对应页面的生命周期。
```

<a id="fn-76cd5dd1fd2ffcb5"></a>

## `io_fence`

```doxygen
@fn inline void io_fence()
@brief RISC-V 特权寄存器和内存/指令可见性：设备 I/Ofence。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 按实际 fence 指令建立设备 I/O 与内存访问的次序约束；这与通过锁互斥访问共享队列是两个不同层面。
```

<a id="fn-1ece543427fe6d3a"></a>

## `icache_fence`

```doxygen
@fn inline void icache_fence()
@brief RISC-V 特权寄存器和内存/指令可见性：icachefence。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@note 配置范围：include-context。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 同步当前 hart 后续取指与此前写入的指令内容。不能把本地 fence.i 当作其他 hart 已经看到新指令的确认。
```
