# `kernel/drivers/tty/core/controlling.cppm`

<a id="fn-712645cfc467fc47"></a>

## `xv::drivers::ControllingTerminal::name`

```doxygen
@fn const char *xv::drivers::ControllingTerminal::name() const override
@brief 返回 `"controlling tty"`。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```

<a id="fn-07dc0ccd74fb2e34"></a>

## `xv::drivers::ControllingTerminal::initialize`

```doxygen
@fn void xv::drivers::ControllingTerminal::initialize() override
@brief 实现 xv::drivers::ControllingTerminal::initialize 的模块操作，按声明参数读写其封装状态。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```

<a id="fn-597766eccc5bafd3"></a>

## `xv::drivers::ControllingTerminal::open`

```doxygen
@fn CharacterDevice *xv::drivers::ControllingTerminal::open(bool) override
@brief 实现 xv::drivers::ControllingTerminal::open 的模块操作，按声明参数读写其封装状态。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```

<a id="fn-667dd971cdbff0ac"></a>

## `xv::drivers::ControllingTerminal::read`

```doxygen
@fn int xv::drivers::ControllingTerminal::read(IoBuffer,int) override
@brief 返回 `-1`。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```

<a id="fn-0c29fbcb71975ee6"></a>

## `xv::drivers::ControllingTerminal::write`

```doxygen
@fn int xv::drivers::ControllingTerminal::write(IoBuffer,int) override
@brief 返回 `-1`。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```

<a id="fn-1312ad1a5a3f2f5b"></a>

## `xv::drivers::ControllingTerminal::put_sync`

```doxygen
@fn void xv::drivers::ControllingTerminal::put_sync(int) override
@brief 实现 xv::drivers::ControllingTerminal::put_sync 的模块操作，按声明参数读写其封装状态。
@details 控制终端工厂只负责打开。group锁内仅固定终端寿命，锁外完成PTY slave打开，避免反向锁顺序。
```
