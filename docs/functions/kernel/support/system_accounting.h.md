# `kernel/support/system_accounting.h`

<a id="fn-a791581b6eedcfdd"></a>

## `xv::stats::Accounting::Accounting<CpuCount>`

```doxygen
@fn constexpr xv::stats::Accounting::Accounting<CpuCount>()=default
@brief 构造对象并建立成员初值及借用关系。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-4ee5c9fcde2a8912"></a>

## `xv::stats::Accounting::timer`

```doxygen
@fn void xv::stats::Accounting::timer(unsigned cpu,bool user,bool active)
@brief 实现 xv::stats::Accounting::timer 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-ab9b05869b631f71"></a>

## `xv::stats::Accounting::external`

```doxygen
@fn void xv::stats::Accounting::external(unsigned cpu)
@brief 实现 xv::stats::Accounting::external 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-31464fc19498ec19"></a>

## `xv::stats::Accounting::dispatch`

```doxygen
@fn void xv::stats::Accounting::dispatch()
@brief 实现 xv::stats::Accounting::dispatch 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-c7032d70a9e9db67"></a>

## `xv::stats::Accounting::created`

```doxygen
@fn void xv::stats::Accounting::created()
@brief 实现 xv::stats::Accounting::created 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-d801a09e72e09916"></a>

## `xv::stats::Accounting::cpu`

```doxygen
@fn bool xv::stats::Accounting::cpu(unsigned index,Cpu &result) const
@brief 实现 xv::stats::Accounting::cpu 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-1c8bfdf08a66351d"></a>

## `xv::stats::Accounting::system`

```doxygen
@fn System xv::stats::Accounting::system() const
@brief 实现 xv::stats::Accounting::system 的模块操作，按声明参数读写其封装状态。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-bc1058d33aa1d768"></a>

## `xv::stats::Accounting::add`

```doxygen
@fn static void xv::stats::Accounting::add(unsigned long &value)
@brief 校验provider名称、只读权限和容量后登记，拒绝重复及保留名称。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```

<a id="fn-eaa5a81a4dd8f66e"></a>

## `xv::stats::Accounting::get`

```doxygen
@fn static unsigned long xv::stats::Accounting::get(const unsigned long &value)
@brief 按inode号复用或取得受引用锁保护的缓存槽。
@details 松弛原子采样计数器。CPU索引有界；快照逐项读取并非全局同一时刻，计时来源为100ms周期。
```
