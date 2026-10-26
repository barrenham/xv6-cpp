# `kernel/process/arch/registers.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/arch/registers.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:registers`。

## 函数导航

- [`xv::process::swtch`](registers.cppm.md#fn-e6e7257048f20d62)

<a id="fn-e6e7257048f20d62"></a>

## `xv::process::swtch`

```doxygen
@fn void xv::process::swtch(Context *old_context, Context *new_context)
@brief 进程对象生命周期、组身份、调度、等待与信号：swtch。
@param old_context 用于保存被切出内核上下文的 Context 存储，对应 a0。
@param new_context 已初始化、准备切入的 Context 存储，对应 a1。
@details 汇编在 a0 指向的旧 Context 中保存 ra、sp、s0..s11，在 a1 指向的新 Context 中恢复同一组寄存器，最后 ret 跳到新 ra。偏移 0/8/16..104 必须与 Context 的布局和 static_assert 一致。
@pre 调用方必须已经满足调度器对进程锁、中断关闭和状态转换的要求。此例程不替调用方获取锁。
@note 这里只切换内核执行上下文，不保存全部用户寄存器，也不切换 satp。用户完整寄存器属于 TrapFrame；跨 hart 的中断恢复意图由 SchedulerContext 维护。
@return 返回到新 Context 的 ra 所描述的位置；旧调用点只有在该旧 Context 再次被恢复后才继续。
```
