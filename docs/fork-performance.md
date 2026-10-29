# fork 的 5 秒性能门槛

`fork_progress_1cpu` 和 `fork_progress_5cpu` 现在是默认 CTest 中的性能门禁。
每种核数在同一次启动中运行 3 轮 `forktest` 和 `usertests forktest`，共 12 个样本。
每个样本都必须通过，不能用平均值或重试后的较快结果覆盖超标样本。

```sh
conda activate OS-LAB
cmake --preset dev
cmake --build build/dev --target fork-performance
```

目标先构建内核及测试镜像，再运行两个门禁。也可以在完成构建后执行：

```sh
ctest --test-dir build/dev --output-on-failure -L performance
```

这次用户要求恢复 fork 性能验证，取代此前“不运行 fork_progress”的维护约束。
之后完整回归应包含它们：`ctest --test-dir build/dev --output-on-failure -j2`。
CTest 将这两个测试标记为 `RUN_SERIAL`，在同一 CTest 调度中独占执行。

## 计量口径

两个用户程序保留原有最多 1000 个子进程的工作负载、逐个 wait 以及最终无子进程检查。
在真实 fork/wait 阶段前后读取 `clockticks()`，输出已创建子进程数和原始时钟差。
门禁使用 QEMU virt 的 10 MHz timebase 转换秒数，并检查：

- 完成 1000 个子进程；内存不足提前结束不能当作性能通过。
- guest 测试主体耗时不大于 5 秒。
- 宿主从主体开始标记到结束标记的耗时不大于 5 秒。
- 独立 `forktest` 从发出 shell 命令到返回提示符的总耗时也不大于 5 秒。

`usertests forktest` 还会在测试前后分别扫描可用内存以检查泄漏。
这些扫描与登录、启动不计入 fork 主体的 5 秒限制，分别记录 setup/cleanup/total，
因此不能将该包装命令的总耗时宣称为 5 秒以内。
阈值比较使用未四舍五入的值；JSON 中 `guest_ticks` 保留原始精度，
显示用的宿主秒数保留三位小数。超标或工作量不足返回非零退出码，功能异常或缺少计量记录也失败。

## 适用环境与证据

标准门禁使用当前构建配置、128 MiB RAM、QEMU virt/TCG、1 核和 5 核。
5 秒是回归失败阈值，不能保证任意宿主硬件、系统负载或任意未来代码都达到它。
建议在没有其他 QEMU 或编译任务争用 CPU 的环境运行；不通过时保留失败结果并排查，不能放宽阈值掩盖回退。
本地门禁不会自行配置 GitHub 分支保护，提交者仍需运行它。

每次运行的计量和控制台日志保存在：

- `build/dev/fork-progress-1cpu.json` 和 `.log`
- `build/dev/fork-progress-5cpu.json` 和 `.log`

JSON 的 `within_budget`、`workload_complete` 必须全部为 true。
这次实测及门禁失败路径验证见 [验收记录](fork-performance-acceptance.json)。

此前家庭链表、退出进度与 fork 风暴停止协议的设计和历史对照数据，保留在 [历史记录](fork-performance-history.md)。
