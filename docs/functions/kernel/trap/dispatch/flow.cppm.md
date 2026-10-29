# `kernel/trap/dispatch/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::trap::TrapDispatcher::DispatchFlow::accepts`](flow.cppm.md#fn-29870242f234bf21)
- [`xv::trap::TrapDispatcher::DispatchFlow::advance`](flow.cppm.md#fn-995417b2176466f2)
- [`xv::trap::TrapDispatcher::DispatchFlow::handle`](flow.cppm.md#fn-00c18dc13ece49b8)
- [`xv::trap::TrapDispatcher::DispatchFlow::missing`](flow.cppm.md#fn-fb9928cdeeadac5b)
- [`xv::trap::TrapDispatcher::DispatchFlow::more`](flow.cppm.md#fn-489323be17611632)
- [`xv::trap::TrapDispatcher::DispatchFlow::table_present`](flow.cppm.md#fn-ea7cd2df889884e9)

<a id="fn-29870242f234bf21"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::accepts`

```doxygen
@fn static inline bool xv::trap::TrapDispatcher::DispatchFlow::accepts(Context &c) noexcept
@brief 验证当前处理器非空并调用其接受条件。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-995417b2176466f2"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::advance`

```doxygen
@fn static inline void xv::trap::TrapDispatcher::DispatchFlow::advance(Context &c) noexcept
@brief 推进处理器表索引。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-00c18dc13ece49b8"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::handle`

```doxygen
@fn static inline void xv::trap::TrapDispatcher::DispatchFlow::handle(Context &c) noexcept
@brief 重新读取当前表项并执行首个接受处理器。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-fb9928cdeeadac5b"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::missing`

```doxygen
@fn static inline void xv::trap::TrapDispatcher::DispatchFlow::missing(Context &c) noexcept
@brief 报告缺失处理器表。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-489323be17611632"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::more`

```doxygen
@fn static inline bool xv::trap::TrapDispatcher::DispatchFlow::more(Context &c) noexcept
@brief 检查是否还有处理器需要检查。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-ea7cd2df889884e9"></a>

## `xv::trap::TrapDispatcher::DispatchFlow::table_present`

```doxygen
@fn static inline bool xv::trap::TrapDispatcher::DispatchFlow::table_present(Context &c) noexcept
@brief 检查处理器表的数量与存储是否一致。
@details 沿处理器表顺序选择第一个接受者；空表有效，非空表必须有存储且每个候选处理器非空。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
