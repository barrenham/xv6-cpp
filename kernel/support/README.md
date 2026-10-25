# 基础服务模块

| 模块 | 公开入口 | 实现 |
| --- | --- | --- |
| `xv.print` | 初始化、printk、panic、panic 状态查询 | `print.cppm` / `printk.cpp` |
| `xv.runtime` | 内存填充/移动/比较，字符串比较/复制/长度，数组长度 | `runtime.cppm` / `string.cpp` |

打印函数和 panic 状态属于 `xv.print`；模块直接导出自己的实现，不再用
`using ::printk` 重导出旧 C 符号。panic 标记为私有原子状态，UART 平台通过
模块查询。引导核先初始化 console，再初始化打印锁，然后发布启动完成状态。

内核代码通过 `import xv.runtime;` 使用 `fill/move/compare_bytes`、
`compare_text/copy_fixed/copy_string/length` 和 `array_size`。
`move` 支持重叠区域，`copy_fixed` 保留固定宽度补零语义，`copy_string`
在容量大于零时保证终止。字节计数使用机器字宽，避免编译器 ABI 的 size_t
被 32 位参数截断。

`compiler_memory.cpp` 只提供编译器生成代码需要的 `memset/memmove/memcpy/memcmp`
标准 ABI 适配，内核模块源码不调用这些全局符号。
`cpp_runtime.cpp` 保留编译器需要的 `__cxa_pure_virtual` 边界并转入打印模块。
两者均不导入宿主机 libc。

`defs.h`、`print_abi.h` 和文件/inode 的 C 引用桥接已删除。共享基础类型、
磁盘/用户 ABI 布局和平台常量继续作为头文件供内核、汇编和用户程序共用。
可选锁实验的统计设备代码归入 `xv.drivers` 实现；本项目默认构建页表实验，
不为缺失的其他实验实现补充占位行为。

`runtime_contract` 在宿主机编译实际运行时模块，检查重叠移动、字节边界、
固定宽度补零、字符串截断与终止，以及零/负容量行为。
