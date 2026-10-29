# `kernel/support/flow.h`

模板流程图的类型、策略和执行器。详细接口、示例与性能约定见
[工具说明](../../../flow.md)。以下条目绑定真实 clangd USR 与函数体指纹。

## 函数导航

- [`xv::flow::action::run`](flow.h.md#fn-5e48447e1869bfb6)
- [`xv::flow::condition::test`](flow.h.md#fn-6e02f403da63e741)
- [`xv::flow::detail::policy<xv::flow::branch<type-parameter-0-0, type-parameter-0-1, type-parameter-0-2>>::evaluate`](flow.h.md#fn-2dc24fbec10e3340)
- [`xv::flow::detail::policy<xv::flow::call<type-parameter-0-0, type-parameter-0-1>>::evaluate`](flow.h.md#fn-682b601f90a5f983)
- [`xv::flow::detail::policy<xv::flow::go<type-parameter-0-0>>::evaluate`](flow.h.md#fn-06ba37248260d48e)
- [`xv::flow::detail::policy<xv::flow::return_>::evaluate`](flow.h.md#fn-33f3537d9571ed27)
- [`xv::flow::detail::policy<xv::flow::stop>::evaluate`](flow.h.md#fn-7d7837c376c9ba28)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute`](flow.h.md#fn-0efcfbe0fc43c0e7)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute_direct`](flow.h.md#fn-74d9cd176f9aaa59)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::index`](flow.h.md#fn-f6d5000a50023dc9)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::run`](flow.h.md#fn-0f7c4a1af3319aab)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::valid_targets`](flow.h.md#fn-72ca13f649ef9864)
- [`xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::visit`](flow.h.md#fn-0dc59d7f2f3f96be)
- [`xv::flow::noop::run`](flow.h.md#fn-7cd48fbd82d32855)
- [`xv::flow::step_budget::take`](flow.h.md#fn-9566d70bba576ab6)
- [`xv::flow::unlimited::take`](flow.h.md#fn-2e97ceaabb41a6f6)

<a id="fn-7cd48fbd82d32855"></a>

## `xv::flow::noop::run`

```doxygen
@fn static void xv::flow::noop::run(Context &) noexcept
@brief 不修改共享上下文的空动作。
@details 不返回业务值；用于纯连接或条件节点。
```

<a id="fn-2e97ceaabb41a6f6"></a>

## `xv::flow::unlimited::take`

```doxygen
@fn bool xv::flow::unlimited::take() noexcept
@brief 无条件允许执行下一个节点。
@details 始终返回 true，不保存计数状态。
```

<a id="fn-9566d70bba576ab6"></a>

## `xv::flow::step_budget::take`

```doxygen
@fn bool xv::flow::step_budget::take() noexcept
@brief 在剩余节点预算内消耗一个执行名额。
@details 剩余为零返回 false；否则递减 remaining 并返回 true。
```

<a id="fn-06ba37248260d48e"></a>

## `xv::flow::detail::policy<xv::flow::go<type-parameter-0-0>>::evaluate`

```doxygen
@fn static transfer xv::flow::detail::policy<xv::flow::go<type-parameter-0-0>>::evaluate(Context &) noexcept
@brief 构造跳转到已验证目标节点的控制描述。
@details 不执行目标节点；索引由 Graph 的节点顺序确定。
```

<a id="fn-682b601f90a5f983"></a>

## `xv::flow::detail::policy<xv::flow::call<type-parameter-0-0, type-parameter-0-1>>::evaluate`

```doxygen
@fn static transfer xv::flow::detail::policy<xv::flow::call<type-parameter-0-0, type-parameter-0-1>>::evaluate(Context &) noexcept
@brief 构造进入子流程及其返回位置的控制描述。
@details 不操作返回栈；由外层迭代执行器检查容量并压栈。
```

<a id="fn-33f3537d9571ed27"></a>

## `xv::flow::detail::policy<xv::flow::return_>::evaluate`

```doxygen
@fn static transfer xv::flow::detail::policy<xv::flow::return_>::evaluate(Context &) noexcept
@brief 构造返回当前子流程调用者的控制描述。
@details 无调用者时由执行器结束并报告 returned。
```

<a id="fn-7d7837c376c9ba28"></a>

## `xv::flow::detail::policy<xv::flow::stop>::evaluate`

```doxygen
@fn static transfer xv::flow::detail::policy<xv::flow::stop>::evaluate(Context &) noexcept
@brief 构造结束整个流程的控制描述。
@details 执行器终止运行并丢弃所有待返回位置，不执行隐式清理路径。
```

<a id="fn-2dc24fbec10e3340"></a>

## `xv::flow::detail::policy<xv::flow::branch<type-parameter-0-0, type-parameter-0-1, type-parameter-0-2>>::evaluate`

```doxygen
@fn static transfer xv::flow::detail::policy<xv::flow::branch<type-parameter-0-0, type-parameter-0-1, type-parameter-0-2>>::evaluate(Context &ctx) noexcept
@brief 求值一次条件并只选择其中一个控制策略。
@details 检查条件返回 bool 且 noexcept；未选中的控制策略不执行。
```

<a id="fn-f6d5000a50023dc9"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::index`

```doxygen
@fn static constexpr unsigned xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::index() noexcept
@brief 将节点身份类型映射为图内连续索引。
@details 不存在的 Id 触发 static_assert；合法索引范围为零到 node_count 减一。
```

<a id="fn-72ca13f649ef9864"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::valid_targets`

```doxygen
@fn static constexpr bool xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::valid_targets(types<Ids...>) noexcept
@brief 检查策略列出的全部目标均属于当前图。
@details 用于编译期验证，空目标列表返回 true；允许自环和回边。
```

<a id="fn-0efcfbe0fc43c0e7"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute`

```doxygen
@fn static detail::transfer xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute(Context &ctx) noexcept
@brief 执行一个节点动作后计算其控制描述。
@details 检查动作返回 void 且 noexcept；业务值由 ctx 保存，不混入控制信号。
```

<a id="fn-0dc59d7f2f3f96be"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::visit`

```doxygen
@fn static void xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::visit(Visitor &visitor)
@brief 按声明顺序向访问器暴露所有节点类型。
@details 调用 visitor.template node<N>()；不执行动作或条件，也不遍历运行时路径。
```

<a id="fn-0f7c4a1af3319aab"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::run`

```doxygen
@fn [[nodiscard]] static outcome xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::run(Context &ctx, Budget budget = {}) noexcept
@brief 迭代执行有向图，按是否含 call 选择单索引或有界返回栈。
@details 不含 call 的图使用单索引分派；含 call 的图使用有界返回栈。返回 stopped、returned、step_limit 或 call_overflow。预算在节点前消耗，调用容量在当前动作后检查；不自动回滚 ctx，也不保存恢复点。
```

<a id="fn-74d9cd176f9aaa59"></a>

## `xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute_direct`

```doxygen
@fn static unsigned xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>::execute_direct(Context &ctx) noexcept
@brief 执行节点并将控制描述编码为下一索引或两种终端索引；所属流程为 xv::flow::graph<type-parameter-0-0, xv::flow::types<type-parameter-0-1...>>。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5e48447e1869bfb6"></a>

## `xv::flow::action::run`

```doxygen
@fn static auto xv::flow::action::run(Context &ctx) noexcept(noexcept(Function(ctx))) -> decltype(Function(ctx))
@brief 调用模板给定静态动作并保留真实返回类型与 noexcept 契约；所属流程为 xv::flow::action。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6e02f403da63e741"></a>

## `xv::flow::condition::test`

```doxygen
@fn static auto xv::flow::condition::test(Context &ctx) noexcept(noexcept(Function(ctx))) -> decltype(Function(ctx))
@brief 调用模板给定的条件并保留真实返回类型与 noexcept 契约；所属流程为 xv::flow::condition。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
