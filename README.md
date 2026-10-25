# xv-labs-cpp-2026

基于相邻的 `xv6-labs-2026` 创建的独立 RISC-V 教学操作系统项目。内核、用户程序和宿主机工具均以 **C++20** 编译；底层启动、上下文切换和系统调用入口仍使用 RISC-V 汇编。构建采用 **CMake + Ninja + Clang/LLD**，IDE 使用 **clangd**。

## 开始使用

```bash
conda activate OS-LAB
cd /home/barrenham/OS/OS-LAB/xv-labs-cpp-2026
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
cmake --build build/dev --target qemu
```

退出 QEMU：先按 `Ctrl-a`，再按 `x`。`qemu` 使用 `build/dev/fs.img`，磁盘修改保留到镜像下次重新生成；测试使用临时磁盘，避免污染开发镜像。

已将 Clang、clangd、LLD、LLVM 工具、CMake、Ninja、Python 安装到 conda 的 `OS-LAB` 环境。首次重建环境可执行：

```bash
conda env update -n OS-LAB -f environment.yml
```

目标工具链是 Clang 自带的 RISC-V 后端，参数为 `--target=riscv64-unknown-elf -march=rv64gc -mabi=lp64d`，不需要额外安装 RISC-V Linux 用户态交叉工具链。Clang 的交叉编译机制见 [官方说明](https://clang.llvm.org/docs/CrossCompilation.html)。

**QEMU 是当前的环境例外**：配置的 conda 渠道没有找到 `qemu` 包，现使用系统已有的 `/usr/bin/qemu-system-riscv64`，验证版本为 9.2.1。运行环境应支持 RISC-V `virt`、Sstc 和 VirtIO 块设备。CMake 先在 conda 中查找 QEMU，再查找系统路径；也可用 `-DXV_QEMU=/path/to/qemu-system-riscv64` 显式指定。

## RAM 与大页直接映射

内核从启动设备树发现实际 RAM，已移除固定 128 MiB 上限和静态页状态数组。默认仍用 128 MiB；可通过 `cmake --preset dev -DXV_RAM_MB=2048` 配置 QEMU 为 2 GiB，然后照常构建、运行。同一个内核 ELF 可接受不同的 QEMU `-m` 容量，不需要为 RAM 长度重新编译内核。

内核 Sv39 直接映射按对齐、长度与权限边界选择 1 GiB / 2 MiB / 4 KiB 叶子，分配器元数据、设备树和固件保留区域会排除出可分配 RAM。`BootMemorySource`、`PageMappingPolicy` 提供抽象接口，`OwnedPage` 与 `OwnedSv39PageTable` 提供 RAII。设计、平台限制与测试数据见 [内存模块说明](kernel/memory/README.md)。

## clangd 与 IDE

clangd 负责补全、跳转、引用和静态诊断；真正执行编译的是 Clang。打开本项目目录后，`.clangd` 自动读取 `build/dev/compile_commands.json`。必须先成功运行一次 configure/build。具名模块需要 CMake 3.28 及以上和 Ninja；当前验证使用 Clang/clangd 23.1.4，环境文件固定了匹配版本。

`.vscode/settings.json` 已配置本机 `OS-LAB` 环境的 clangd 路径，使用 CMake 已生成的 BMI，并关闭重复的 C/C++ IntelliSense。默认不启用 clangd 的实验性模块自动构建器。如果打开的是上层 `OS` 工作区，使用上层 `.vscode/settings.json` 的配置；子目录设置不会自动成为根工作区设置。迁移到另一台机器时修改该文件中的两个 conda 路径。宿主机 `mkfs` 有独立的 `build/dev/host/compile_commands.json`，`.clangd` 按文件路径选择数据库，避免把宿主机程序当作 RISC-V 内核解析。

可手动验证配置：

```bash
clangd --check=kernel/memory/paging/vm.cpp --check-locations=false --tweaks=
clangd --check=user/tests/pgtbltest.cpp --check-locations=false --tweaks=
clangd --check=tools/mkfs/main.cpp --check-locations=false --tweaks= \
  --query-driver="$CONDA_PREFIX/bin/clang++"
```

## C++20 具名模块

物理页分配器对外统一使用：

```cpp
import xv.memory;
```

公开入口为 `kernel/memory/memory.cppm`，抽象接口位于 `kernel/memory/allocator/page_allocator.cppm`。`allocator/buddy/` 和 `allocator/freelist/` 分别放置两种具体实现；实例选择和共享辅助定义放在 `allocator/`。普通调用方不依赖这些内部模块分区。完整目录树见 [内存模块说明](kernel/memory/README.md#从目录找代码)。

实现文件通过 `module xv.memory;` 与 `import :detail;` 等声明访问同一模块的内部定义。CMake 的 `CXX_MODULES` 文件集负责依赖扫描和 BMI 编译，无需手工维护编译顺序。RISC-V 内核和宿主机测试各自编译模块产物，不共享 `.pcm` 文件。

模块化覆盖地址、内存布局、启动内存发现、物理页分配器和 Sv39 页表。原有 VM 集成、驱动等接口仍保留 C/C++ 兼容头文件；RISC-V 汇编继续使用预处理头文件。模块不会导出宏，因此 `layout/memlayout.h` 的平台宏只在模块边界转成类型明确的常量。内核不使用 `import std`，保持 freestanding 构建。

当前 IDE 使用构建系统生成的模块文件和编译命令，不让 clangd 自动重建模块。此前自动构建路径在同一编辑器进程中出现过模块依赖重建失败与类型未声明的误报。新增或修改 `.cppm` 后，先运行 `cmake --build --preset dev` 更新内核及宿主机 BMI，再执行 `clangd: Restart language server` 刷新已打开文件；普通 `.cpp` 的编辑仍由 clangd 实时分析。修改编译器版本后需重新配置和构建。可通过实际 LSP 请求检查诊断、跨模块悬停、跳转和成员补全：

```bash
python tests/clangd_modules.py --clangd "$CONDA_PREFIX/bin/clangd"
```

此检查只使用未保存的编辑缓冲区，不修改源文件。

## 目录原则

采用按职责组织的包目录，接口和实现放在同一个子系统中，不将所有头文件另堆到一个全局目录。

| 目录 | 职责 |
| --- | --- |
| `kernel/arch/riscv` | RISC-V 寄存器与指令辅助函数 |
| `kernel/boot` | 启动和多核初始化 |
| `kernel/memory` | 页分配、伙伴分配器、页表、地址布局 |
| `kernel/memory/address` | 物理地址和区域值对象 |
| `kernel/memory/layout` | 可用区域规划与平台地址常量 |
| `kernel/memory/boot` | RAM 发现、分配器启动集成与 C ABI 适配 |
| `kernel/memory/allocator` | 抽象分配接口、实例服务及 buddy/freelist 实现目录 |
| `kernel/memory/paging` | Sv39 页表、映射策略与地址空间集成 |
| `kernel/process` | 进程、调度、上下文切换、ELF 加载 |
| `kernel/trap` | 中断、异常、用户态与内核态切换 |
| `kernel/syscall` | 系统调用 ABI、分发和处理函数 |
| `kernel/fs` | inode、文件、管道、日志和块缓存 |
| `kernel/sync` | 自旋锁、睡眠锁、RAII guard |
| `kernel/drivers` | UART、console、PLIC、VirtIO |
| `kernel/support` | 基础类型、字符串、打印和兼容接口声明 |
| `user/apps/<程序名>` | 每个应用独立目录 |
| `user/runtime` | 系统调用入口、程序入口、分配器、打印和 C++ 资源封装 |
| `user/tests` | 操作系统回归及页表测试程序 |
| `tools/mkfs` | 独立的宿主机文件系统镜像工具 |
| `linker` | 内核和用户程序链接布局 |
| `cmake` | 交叉工具链配置 |
| `tests` | 宿主机驱动的 QEMU 集成测试 |

每个内核子系统维护自己的 `CMakeLists.txt`。新增应用时，在 `user/CMakeLists.txt` 的 `programs` 中登记路径。`migration-map.json` 记录旧路径到新路径的对应关系，便于对照教材。

## C++ 使用边界

- `PhysicalAddress` 封装地址值与对齐、页号计算；`PhysicalMemoryRange` 封装区域边界与区域内页索引，地址不会隐式转换为整数。
- `PhysicalPageAllocator` 提供统一的物理页分配接口，伙伴和链表实现均在启动时可选；唯一实例经显式构造后固定使用。所有内存分配调用点已迁移到抽象接口，详见 [内存模块说明](kernel/memory/README.md)。
- `SpinLock`、`SleepLock` 将状态和操作合并为类，所有调用点已迁移为成员调用；`xv::sync::ScopedLock` 在文件表操作中处理正常返回和提前返回时的解锁。
- `xv::user::File` 管理文件描述符，支持析构释放和移动所有权，禁止复制。`cppdemo` 连续执行超过描述符上限的打开/移动/析构操作，验证不会泄漏。
- 文件类型和用户态打开模式采用 `enum class`，显式表达类型；按已确认的选择，不启用 `dynamic_cast`、`typeid` 和完整 RTTI。
- 不链接宿主机 libc、libstdc++、libc++、异常运行时或启动对象。目标文件直接交给 LLD；`mkfs` 是独立宿主机程序，允许使用宿主机标准库。
- 内核和用户态禁用异常、RTTI、线程安全静态初始化、自动全局析构。链接脚本拒绝需要执行全局构造/析构的 `.init_array`、`.fini_array`。
- 普通栈对象的构造/析构、模板、引用、枚举、命名空间和 RAII 可以使用；提供了不分配内存的 placement new，用于启动期实例构造；暂未提供通用 `new/delete`、STL、线程局部存储或异常展开支持。需要动态分配时仍使用现有显式分配接口，并检查失败。
- 锁在启动阶段显式 `init()`。RAII 锁不能跨越 `sched()`、`sleep()` 或其他转移锁所有权的边界，这些路径保留显式锁操作。
- 迁移保留了原有进程、虚拟内存和文件系统算法，以及用于对照实验的 C ABI 接口。并非将每个全局函数都包装为类；`kernel/support/defs.h` 仍是兼容声明汇总。后续类化应围绕资源所有权和不变量逐步进行。

为兼容原有实验接口，目前保留部分 `char *` 字符串参数，并关闭字符串字面量转为 `char *` 的诊断；这些字面量不得被修改。新增接口应使用 `const char *`。

## 验证与调试

```bash
# 快速启动、C++ RAII、页表回归
ctest --preset dev -R boot_smoke --output-on-failure
# 原有用户程序的快速回归集
ctest --preset dev -R user_regression --output-on-failure
# 启动后停在首条指令，GDB 端口默认为 26000
cmake --build build/dev --target qemu-gdb
```

GDB 使用支持 RISC-V 的版本，加载 `build/dev/kernel.elf`，执行 `target remote :26000`。可在配置时通过 `-DXV_CPUS=1` 或 `-DXV_GDB_PORT=26001` 调整参数。

`kernel.elf` 的入口必须为 `0x80000000`，用户态 `_user_start` 调用 `main` 后将返回值交给 `exit`。LLD 的用户程序加载段已显式设置，满足 xv6 的页对齐要求。没有动态链接器或未解析的运行时符号。

## 来源与实验范围

来源：相邻项目 `xv6-labs-2026` 的当前干净工作树，提交 `6ba92da7496318fb9b011c4d0dfb52a5712ae51d`，原配置 `LAB=pgtbl`。保留原有页表实验行为与已有实现，没有新增实验答案；原项目未修改。

本项目固定构建当前页表快照，不宣称兼容其他实验分支。原来的 Makefile 评分、提交脚本依赖旧路径，未作为可用的 CMake 评分入口迁入；这里的 CTest 是运行验证，不替代课程评分。

保留原 MIT 许可于 `LICENSE`，上游说明于 `README.xv6`。此项目不需要 GitHub 连接。

本机验证记录（2026-10-25）：Clang/clangd/LLD 23.1.4，CMake 4.3.1，QEMU 9.2.1。C++20 构建在启用 `-Werror` 下通过；默认回归、原生契约测试与可变 RAM 测试均通过，后者运行完整的 `usertests -q` 并输出 `ALL TESTS PASSED`。三类源文件的 clangd 检查均为零错误；26 个目标可执行文件均无未定义符号。再次增量构建不会重建未变化的磁盘镜像。
