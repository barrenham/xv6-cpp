# 同步模块

所有 C++ 调用方统一使用 `import xv.sync;`，不再包含锁的旧头文件。

```cpp
import xv.sync;

void update(xv::sync::Lock &lock)
{
  xv::sync::ScopedLock guard(lock);
  // 临界区，正常返回和提前返回都会释放锁。
}
```

`Lock` 是不可复制的抽象接口，定义 `init`、`acquire`、`release`、
`holding` 和 `kind`，并保存调试名称。`SpinLock`、`SleepLock` 是 `final`
派生类；调用方可用 `LockKind` 查询类型，无需 RTTI。`ScopedLock` 保存
`Lock &` 并执行虚调用，能够统一管理两种锁。具体锁仍由使用它的子系统拥有，
不需要全局锁注册表或动态分配锁对象。

| 文件 | 职责 |
| --- | --- |
| `sync.cppm` | 模块入口，重导出公共接口分区 |
| `lock.cppm` | 抽象接口及 `LockKind` |
| `spin/spinlock.cppm`、`spin/spinlock.cpp` | CPU 所有权、自旋及 acquire/release 内存序 |
| `sleep/sleeplock.cppm`、`sleep/sleeplock.cpp` | 进程所有权、等待及唤醒；内部用自旋锁保护状态 |
| `scoped_lock.cppm` | 基于抽象接口的作用域守卫 |
| `platform/platform.cppm` | 内部平台操作契约，不向调用方导出 |
| `platform/kernel_platform.cpp` | RISC-V 中断嵌套与 CPU / 进程适配 |
| `platform/kernel_bridge.cpp` | 模块实现与现有 C ABI 的桥接 |

## 语义与生命周期

- `SpinLock` 禁用本 CPU 中断，自旋至获得锁；释放后恢复嵌套中断状态。
  `holding()` 在内核中要求调用时中断已关闭，避免检查途中迁移 CPU。
- `SleepLock` 可阻塞，只用于进程上下文。等待时先登记通道，再释放内部
  自旋锁并睡眠；唤醒方持内部锁修改状态，沿用现有协议防止丢失唤醒。
  `holding()` 自行锁定内部状态，检查当前进程是否持锁。
- 两种锁均拒绝递归获取和非所有者释放。锁不提供公平性保证。
- 守卫不能与手动释放或调度器的锁所有权交接混用。普通自旋锁临界区不能
  跨越 `sleep()` / `sched()`；睡眠锁守卫可以包围阻塞操作。
  调度器的 `p->lock` 有专门的上下文交接协议，继续显式获取和释放。
- 锁在发布给其他 CPU / 进程前调用 `init(label)`；禁止在使用中重新初始化。
  `constexpr` 默认构造安装虚表，含锁的全局对象使用 `constinit`，数组及
  聚合对象显式 `{}` 初始化，避免生成启动构造函数。链接脚本继续拒绝
  `.init_array` / `.fini_array`。
- 物理页只提供原始存储。管道中的锁通过 `new (pi) pipe` 开始对象生命周期，
  归还页面前调用 `pi->~pipe()`。今后新增此类对象也必须显式构造。
- `Lock` 的析构函数受保护且非虚，禁止经基类指针删除对象；容器按具体类型
  管理生命周期。内核不依赖堆分配、异常、RTTI 或标准库。

`proc.h`、`file.h`、`buf.h` 等现有聚合类型头文件使用模块中的具体锁类型，
包含它们的 C++ 翻译单元应先导入 `xv.sync`。模块实体不再在 `defs.h`
中声明为全局类。共享计时器锁的声明位于 `kernel/trap/ticks.h`。
用户态 `init` 仅包含设备编号头文件，不再依赖内核锁及 inode 布局。

## 验证

`sync_contract` 编译实际两种锁实现，测试抽象接口、RAII 提前返回、
多线程竞争、所有权检查、误用诊断及原始存储构造。
宿主机仅替换 CPU、中断和等待/唤醒平台操作；分配器契约测试也使用相同
实际锁实现。QEMU 启动与 `usertests -q` 验证真实多核调度和设备路径。

```bash
ctest --preset dev --output-on-failure
ctest --preset freelist --output-on-failure
python tests/clangd_modules.py --clangd "$CONDA_PREFIX/bin/clangd"
```
