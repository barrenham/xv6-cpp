# 系统可靠性验收：2026-11-08

**后续状态：下述文件名兼容缺陷已修复，见文末修复复测。原始 77 项检查及 10 分钟采样保留为修复前记录，不表示本次重新运行了全部验收。**

本轮结论为**部分通过，不能判为全项通过**。默认 ext3 配置的选定运行路径、故障恢复和 10 分钟混合负载通过；旧 xv6 格式的安装清单与文件名限制不匹配，阻断该格式的打包及更新验收。没有为取得通过结果而修改生产代码或放宽现有断言。

## 版本与方法

- v0.0.4，提交 `5c3224ba2eeca3c038b44eb13ee175e8c631fbfb`。
- QEMU emulator version 9.2.1 (Debian 1:9.2.1+ds-1ubuntu5.2)；RISC-V virt，128 MiB，单核/五核。
- 串行执行，测试进程 nice=10、ionice idle，不并行运行多个 QEMU。
- 选取 45 项原生/主机合约和 32 项 QEMU 集成检查，共 77 个独立用例；**不是整个 245 项测试集的全量运行**。
- 所有客户机均使用测试种子的独立磁盘副本。验收前后核对 SHA-256：kernel、bootstrap ELF、测试种子和用户持久根盘均未改变。

## 汇总

| 检查 | 结果 |
| --- | --- |
| 原生/主机合约（重试后按唯一用例统计） | 44/45 通过 |
| 单核/五核集成 | 32/32 通过 |
| 五核、同一次启动混合负载 | 600.238 秒，8 轮，通过 |
| 混合负载后的重启、文件核对、正常关机及只读 e2fsck | 通过 |

集成覆盖内存耗尽、ext3 部分写入与挂载快照、网络/HTTP、运行时调度切换、设备热插拔、重启、POSIX IPC、Unix socket、文件并发、文件映射/COW、页缓存回收、服务管理器故障恢复、FS 线程池和内核线程/pthread。原生检查还包含 JBD 写入中断/恢复等路径；这不等于真实硬件断电验收。

网卡在单核/五核下各完成 24 轮插拔，拔除后可用内存分别始终为 124944 KiB / 123688 KiB。服务检查主动杀死管理器并验证接管、失败预算、日志轮转和重复实例拒绝；它不承诺根 FS 后端崩溃后的自动重挂载恢复。

## 混合负载与资源回收

每轮启动调度切换压力程序、ping 和本地 HTTP 下载后台任务，前台执行 FS 线程池读写基准，然后等待工作进程结束、同步文件系统并采样。不同阶段允许重叠，不保证所有负载在整轮内持续同时活跃。HTTP 只访问宿主机上的测试服务器，不依赖公网。

每轮基准报告 4 MiB 文件读写，并下载 1 MiB 数据；共报告 32 MiB 基准读写和 8 MiB HTTP 下载，不含预置、额外回读及元数据开销。每轮验证基准内容、调度程序结果、ping 和下载长度；最后重启并正常关机后，直接读取私有磁盘，将最终下载文件与原始字节逐一比较。

| 回收后采样指标 | 观测 |
| --- | --- |
| 第 2～8 轮可用内存 | 123596 KiB |
| 第 2～8 轮报告的文件页缓存 | 204 KiB |
| 第 2～8 轮扣除文件页缓存后的已用内存 | 5140 KiB |
| 预热后首尾非文件页缓存内存漂移 | 0 KiB |
| 任务数（包含采样程序） | 21～21 |
| 进程数（包含采样程序） | 13 |
| 页缓存待处理页 | 各轮均为 0 |

第一次采样非文件页缓存内存为 5132 KiB，之后增加 8 KiB 并保持稳定。预先设定的判据为：排除前两轮预热后，首尾各三轮最小值的内存漂移不超过 1024 KiB，任务数漂移不超过 4。本轮实际漂移均为 0。采样发生在每轮工作任务结束后，测量的是回收后状态，**不是峰值内存，也不能证明不存在任何泄漏**。

## 未通过项：旧格式安装清单

`rootfs_update_contract` 在修正运行环境后仍失败：

```text
rootfs-update: invalid or duplicate installation path
```

直接使用当前清单创建全新的私有 xv6 镜像也失败：

```text
mkfs: invalid image path component
```

原因：旧格式每个文件名组件最多 14 字节，而同一份 `fs-manifest.tsv` 包含：

| 内部测试程序名 | 字节数 |
| --- | --- |
| demandcachetest | 15 |
| fsconcurrencytest | 17 |
| fsparallelbench | 15 |
| schedswitchtest | 15 |

`legacy-test-image` 是 `ALL` 构建目标，直接使用该清单。这意味着问题不只是一个旧测试文件：重新生成该镜像的打包命令确实无法完成，并会阻断包含该目标的全量构建。本轮直接复现了对应 mkfs 命令，没有重新执行整个 ALL 构建。

默认 ext3 支持这些名称，本轮没有观察到其运行中数据损坏。旧格式更新用例在清单校验时已退出，后面的回滚/保护属性检查不能算作已验收。修复方向应是提供符合旧格式的名称映射和相匹配的清单、种子，再完整复测，不能简单删除文件名校验。

## 测试控制修正与证据

- 首轮 `vm_stack_contract` 因验收启动环境的 PATH 缺少 ninja 而未执行；修正 OS-LAB 环境后所有诊断探针通过。这是测试环境问题。
- 首次混合测试因将后台作业重绘提示符误认为新命令确认而中断。脚本增加对应作业的明确启动确认后，在**同一 OS 二进制**上重新完成完整窗口。首次日志保留，不计作完成的耐久测试，也不据此宣称内核故障。

[结构化结果](reliability-acceptance-results.json) 记录最终唯一用例结果、重试历史、构建哈希、逐轮资源数据及保护校验。原始日志、JUnit、私有测试磁盘及 e2fsck 结果保存在 `build/reliability-20261108/`：

- `native.xml`、`native-retry.xml`、`integration.xml`：全部用例及初次失败输出。
- `legacy-reproduction.log`：全新旧格式镜像的独立打包复现。
- `soak-r1/`：中断的测试控制尝试。
- `soak-r2/result.json`、`console.log`、`e2fsck.log`、`root.img`：完整混合测试证据。
- `preservation.json`：验收前后镜像与内核哈希一致性。

## 重复混合测试

先激活 OS-LAB 环境并构建 kernel/test-image，确保没有其他 QEMU 与测试竞争。输出目录必须不存在：

```sh
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev --target kernel test-image -j2
nice -n 10 ionice -c3 python3 tests/reliability_soak.py \
  --qemu qemu-system-riscv64 --kernel build/dev/kernel.elf \
  --bootstrap build/dev/user/image/_fsboot --image build/dev/fs-test.img \
  --output build/reliability-soak-new --cpus 5 --seconds 600
```

本次没有修改生产代码，也未更新用户持久根盘。结论限于当前 QEMU 配置、选定负载和有界窗口；未覆盖真实硬件、数十小时运行、所有磁盘满/驱动故障组合或完整 POSIX 兼容性认证。

## 修复复测（2026-11-08）

旧格式文件名兼容缺陷已修复。沿用已有内部测试的安装别名机制，将四个测试安装为 demandcache、fsconcurtest、fsparbench、schedswitch；源文件、构建目标及诊断输出保留原名。测试脚本和分析工具同步调用短路径。CMake 对新增内部测试检查安装名长度，超限时提示添加显式别名；没有截断名字、丢弃测试或放宽旧格式校验。

复测结果：

- 真正执行 `cmake --build build/dev -j2` 的 ALL 构建成功，ext3 和 xv6 镜像均生成成功。
- 两种测试镜像中的全部 150 个清单文件与源文件逐字节匹配。
- 原先失败的 rootfs_update_contract 通过，并增加了超长名称仍须拒绝、原盘不变的负向检查。
- 镜像生成、配置保护、五核客户机更新、单核/五核旧格式启动，以及新路径下的调度、文件并发、FS 线程池、服务恢复回归通过，共 10 项。
- 栈诊断改为接收 CMake 找到的 Ninja 绝对路径；在 PATH 仅含 /usr/bin:/bin、没有 CONDA_PREFIX 的环境下也通过，合计 11 项针对性检查。
- 单独验证旧格式拒绝超长组件且保留原镜像，ext3 仍能创建并读取相同的长名称。

已向开发持久根盘增加四个短名称，149 个非默认配置清单文件核对正确；账户和 services.conf 与更新前备份一致。旧长名称按更新工具的清单外文件保护规则保留。此步骤属于修复阶段，不改变前文“原验收未修改持久盘”的历史记录。

复测证据在 build/legacy-name-fix/，包括 full-build.log、validation.xml、minimal-path-vm.xml、packaging-checks.json、rootfs-update.log 和 summary.json。原验收脚本快照保存在 build/reliability-20261108/reliability_soak_original.py；当前脚本已更新为短安装路径。原始 10 分钟数据不覆盖本次打包改动，也没有被重新计为执行过。
