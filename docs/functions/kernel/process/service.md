# 公开进程服务与管理器的连接

本文是独立的函数级开发说明；Doxygen 内容由工具写入 clangd 外部索引，不注入源码。

<a id="service-initialize"></a>

## `xv::process::initialize`

```doxygen
@fn void xv::process::initialize()
@brief 使用构建默认值初始化进程服务，或在启动时显式选择调度策略。
@details 无参数入口调用带 SchedulerKind 的启动选择接口；该接口先验证枚举，再以 CAS 拒绝重复初始化，绑定并初始化就绪队列与原有进程服务，最后 release 发布实际选择。带参数入口失败返回 false；无参数入口遇到失败 panic。调度开始后不支持热切换。
```

<a id="service-fork-process"></a>

## `xv::process::fork_process`

```doxygen
@fn int xv::process::fork_process()
@brief 通过公开进程服务入口创建当前进程的子进程。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="service-exit-process"></a>

## `xv::process::exit_process`

```doxygen
@fn void xv::process::exit_process(int status)
@brief 通过公开服务入口结束当前进程的普通执行。
@param status 项目普通退出状态。
@return 不返回。
@details 转交 process_manager.exit_process(status)，显式信号参数使用默认 0。显式信号终止走管理器内部带 signal 参数的路径。
@see ProcessManager::exit_process
```

<a id="service-wait"></a>

## `xv::process::wait`

```doxygen
@fn int xv::process::wait(unsigned long address)
@brief 等待任一子进程退出，保留旧 xv6 普通状态 ABI。
@param address 用户态 int 状态地址；零表示只回收而不复制状态。
@return 成功返回子 PID；无子进程、复制失败或等待被中断时返回负值。
@details 转交 process_manager.wait(address)。停止/继续事件与位编码由另一入口 wait_for_child 提供；不要把旧 wait 的普通状态当作 WIFEXITED 输入。
@see xv::process::wait_for_child
```

<a id="service-wait-for-child"></a>

## `xv::process::wait_for_child`

```doxygen
@fn int xv::process::wait_for_child(int pid, unsigned long address, int options)
@brief 提供支持 PID/组选择和停止、继续事件的 waitpid 服务。
@param pid 正值选择 PID，-1 选择任一子进程，0 选择当前组，小于 -1 选择指定组。
@param address 用户状态输出地址；零表示不输出状态。
@param options WNOHANG、WUNTRACED、WCONTINUED 的组合。
@return 有事件返回子 PID；WNOHANG 无事件返回 0；错误或中断返回负值。
@details 转交 process_manager.waitpid，使用 wait.h 位编码。只有退出事件会回收对象；停止和继续只消费一次事件标记。
```

<a id="service-wakeup"></a>

## `xv::process::wakeup`

```doxygen
@fn void xv::process::wakeup(void *channel)
@brief 向公开进程服务提交对象通道唤醒。
@param channel 对象等待条件地址。
@return 无返回值。
@details 转交 process_manager.wakeup。驱动、文件系统等通过此入口隐藏等待表实现；普通等待与 poll 都由通道地址匹配。
@see WaitChannels::wake
```
