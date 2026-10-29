# fork 压测性能与退出进度

本次修改针对 `forktest`、`usertests forktest` 和高并发 fork/退出路径。
两个 forktest 的 1000 次上限、reparent2 的 800 轮及 fork 风暴持续时间均保留。
相同镜像对照测量和本次 17 项联合检查均已完成。

## 实现

- Process 增加由 ProcessManager 家庭锁保护的双向子链表，包含尚未回收的 Zombie。
  fork 在发布 Runnable 前追加子进程；reap 常数时间脱链；reparent 只遍历自己的直接子进程。
  wait/waitpid 只扫描自己的子进程，不取得全局登记表锁，不逐个锁住无关进程。
  子进程按创建/收养顺序追加；wait 任意子进程时允许选择任一符合条件的事件。
- 家庭锁稳定链表和对象生命周期，子进程锁保护退出、停止和继续事件。
  未发布构造失败对象未挂接，脱链为空操作。原有 PID/进程组筛选、WNOHANG、
  WUNTRACED、WCONTINUED 和用户状态复制失败不消费事件的约定保留。
- SIGCHLD 仍按处置规则登记 pending；默认、忽略或被屏蔽且没有可中断信号时，
  跳过延迟全进程信号唤醒扫描。wait/waitpid 的家庭事件通知独立保留。
  父进程修改处置/屏蔽字时正在运行，进入可中断睡眠前仍在进程锁下复查 pending。
- 陷阱入口检查当前栈所需的最低余量页。栈映射保持连续后缀，已映射时跳过
  全局栈锁和重复的逐页检查；缺页时仍走原串行增长路径。
  当前运行进程或永久调度器栈稳定对应叶表生命周期。

这些改动没有为进程增加硬性数量限制。每个描述符仍使用原来的一页存储，
新增私有链接没有增加分配页数；实际空间约束仍由 PagedProcessStore 的编译期断言检查。
没有保留诊断打印或额外的 fork 分配重试。

## fork 风暴的停止条件

优化使进程创建更快后，原 forkforkfork 压测暴露出停止协议的问题：
工作进程通过 `open("stopforking", 0)` 判断停止文件是否存在。
动态进程数量可以超过全局 NFILE=100，文件槽耗尽会让已有文件的 open 返回失败，
工作进程误判文件不存在并继续 fork，继而持续占满物理页。
隔离诊断内核确认失败发生在 sys_open 的文件对象/描述符分配分支（files.allocate 或 fdalloc），停止文件的存在性不能由这类失败判断。

存在性检查改为已有的 `access("stopforking", F_OK)`，不占文件槽。
创建停止文件的路径、fork 循环、持续时间、后续回收和空闲页检查都保留。
这让停止协议适用于当前动态进程登记模型，避免用正常文件槽耗尽错误驱动无限 fork。

## 测量方法

[tests/fork_progress.py](../tests/fork_progress.py) 使用独立测试镜像副本和 QEMU snapshot，
登录后分别执行原有的 `forktest` 与 `usertests forktest`，记录：

- 命令准备时间：直到实际测试开始标记；
- 测试本体时间：从开始标记到测试成功标记；
- 清理时间：直到 shell 返回；
- 命令总时间：不含本次启动和登录。

`usertests forktest` 在测试前后各调用 countfree，通过逐页分配到内存耗尽再释放来检查泄漏。
这两次检查的耗时不等于 fork 测试本体的耗时，所以报告分段数字。
修复前后使用相同更新后的测试镜像、128 MiB RAM、同一个 QEMU 与 dev 编译配置；
比较内核分别为本次优化前保存的 ELF 和最终构建 ELF。
测量不与其他本次启动的 QEMU 测试并行；主机负载仍会影响结果，不设置脆弱的速度阈值。

```sh
python tests/fork_progress.py --qemu /usr/bin/qemu-system-riscv64 \
  --kernel build/dev/kernel.elf --image build/dev/fs-test.img \
  --cpus 1 --repeat 2 --output build/dev/fork-timing.json
```

CTest 的 fork_progress_1cpu 与 fork_progress_5cpu 同时保留两种原有入口的功能验证，
检查正常完成、等待错误和完整空闲页检查结果，并生成 JSON 分段计时和终端日志。

## 本次结果

QEMU virt、128 MiB、dev 的同镜像对照。修复前单核两轮、五核一轮，修复后各一轮；
数据为本机墙钟时间，保留样本范围，未将其外推为固定硬件倍率。

| 入口 | CPU 数 | 本体修复前 | 本体修复后 | 命令总耗时修复前 | 命令总耗时修复后 |
| --- | ---: | ---: | ---: | ---: | ---: |
| forktest | 1 | 6.503–6.936 s | 3.665 s | 6.575–7.391 s | 3.937 s |
| usertests forktest | 1 | 6.525–7.460 s | 3.845 s | 24.062–27.398 s | 22.262 s |
| forktest | 5 | 6.784 s | 3.380 s | 6.935 s | 3.494 s |
| usertests forktest | 5 | 7.414 s | 3.418 s | 28.102 s | 20.600 s |

修复后 usertests 的两次 countfree 仍分别耗时约 8.5–9.4 秒。
没有取消这两次内存检查，也没有缩小 1000 次 fork 的测试上限。
分段数据见运行产物 `build/docs/fork-performance/comparison.json`。
内核 text/只读区由 270976 B 变为 270928 B，data 和 bss 保持不变。

17 项联合检查通过：两种 fork 入口的单核/五核测试、既有 user_regression（usertests -q）、
同步契约、进程/存储/组/信号契约、单核/五核 SIGCHLD、信号、进程组和 96 子进程扩展，
以及验证锁内栈增长、权限、应急帧耗尽和 guard 的四种私有内核栈探针。
另外独立执行 forkforkfork 与 reparent2，两者及前后空闲页检查均通过。

真实 clangd 文档覆盖、悬浮、跳转和源码完整性检查通过；新增两个家庭链表辅助方法也已登记。
`kernel/boot` 的 8 个文件内容和文件集合保持不变。交付内核不包含 FORK_DEBUG/STOP_DEBUG 诊断代码。
