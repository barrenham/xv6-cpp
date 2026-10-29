# `kernel/net/filter/flows/engine.h`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::filter::Engine::CommandFlow::append`](engine.h.md#fn-4d799c3c533c493c)
- [`xv::net::filter::Engine::CommandFlow::create`](engine.h.md#fn-05e12f4cb3f75de2)
- [`xv::net::filter::Engine::CommandFlow::delete_chain`](engine.h.md#fn-5dbfe5bf884493ac)
- [`xv::net::filter::Engine::CommandFlow::flush`](engine.h.md#fn-69554bb5871cc51b)
- [`xv::net::filter::Engine::CommandFlow::found`](engine.h.md#fn-4d6886553411a1d8)
- [`xv::net::filter::Engine::CommandFlow::insert`](engine.h.md#fn-aa281b870971c1fa)
- [`xv::net::filter::Engine::CommandFlow::inspect`](engine.h.md#fn-6f5f651c97674a7e)
- [`xv::net::filter::Engine::CommandFlow::invalid`](engine.h.md#fn-89ffdeb9ea49342b)
- [`xv::net::filter::Engine::CommandFlow::lookup`](engine.h.md#fn-25499c80890e1a05)
- [`xv::net::filter::Engine::CommandFlow::missing`](engine.h.md#fn-aaf4611bd7b006e3)
- [`xv::net::filter::Engine::CommandFlow::operation`](engine.h.md#fn-fca01367051ffb20)
- [`xv::net::filter::Engine::CommandFlow::policy`](engine.h.md#fn-333796d87ef84038)
- [`xv::net::filter::Engine::CommandFlow::remove`](engine.h.md#fn-c517669ffe9497c0)
- [`xv::net::filter::Engine::CommandFlow::rule`](engine.h.md#fn-869eb132fbce0578)
- [`xv::net::filter::Engine::CommandFlow::valid`](engine.h.md#fn-e564c94606a567cf)
- [`xv::net::filter::Engine::CommandFlow::valid_name`](engine.h.md#fn-5cfed7eae5e37c38)
- [`xv::net::filter::Engine::CommandFlow::valid_policy`](engine.h.md#fn-157128eef202c6c0)
- [`xv::net::filter::Engine::CommandFlow::zero`](engine.h.md#fn-23eeca5fea8571b4)
- [`xv::net::filter::Engine::EvaluationFlow::count`](engine.h.md#fn-9273219bb4415e84)
- [`xv::net::filter::Engine::EvaluationFlow::fetch`](engine.h.md#fn-376951f51cb8cf2e)
- [`xv::net::filter::Engine::EvaluationFlow::has_caller`](engine.h.md#fn-69d69941cfebeb05)
- [`xv::net::filter::Engine::EvaluationFlow::jump`](engine.h.md#fn-c129d45f9ce7c827)
- [`xv::net::filter::Engine::EvaluationFlow::lookup`](engine.h.md#fn-409d46f3ed0c31ce)
- [`xv::net::filter::Engine::EvaluationFlow::match`](engine.h.md#fn-4222f8785a1bb6f4)
- [`xv::net::filter::Engine::EvaluationFlow::more_rules`](engine.h.md#fn-31ed373078ed9e41)
- [`xv::net::filter::Engine::EvaluationFlow::policy`](engine.h.md#fn-7040950a9790dd73)
- [`xv::net::filter::Engine::EvaluationFlow::pop`](engine.h.md#fn-0e28cb0897f52a5e)
- [`xv::net::filter::Engine::EvaluationFlow::push`](engine.h.md#fn-63f232df0d8a8282)
- [`xv::net::filter::Engine::EvaluationFlow::root_valid`](engine.h.md#fn-68bcc8bfe72a417e)
- [`xv::net::filter::Engine::EvaluationFlow::select_root`](engine.h.md#fn-34851c0233d7760f)
- [`xv::net::filter::Engine::EvaluationFlow::terminal`](engine.h.md#fn-6031965f070aeda9)
- [`xv::net::filter::Engine::EvaluationFlow::valid_target`](engine.h.md#fn-c40deed0388ed2ef)
- [`xv::net::filter::Engine::EvaluationFlow::verdict`](engine.h.md#fn-d75e7924ac6bbb23)
- [`xv::net::filter::Engine::EvaluationFlow::within_budget`](engine.h.md#fn-6e8b3e3214b004f0)
- [`xv::net::filter::Engine::accept`](../engine.h.md#fn-d5fed1d71441705c)
- [`xv::net::filter::Engine::execute`](../engine.h.md#fn-286f6e66a00e3c34)

<a id="fn-4d799c3c533c493c"></a>

## `xv::net::filter::Engine::CommandFlow::append`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::append(Context &c) noexcept
@brief 追加已验证的数据、路由或对象记录；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-05e12f4cb3f75de2"></a>

## `xv::net::filter::Engine::CommandFlow::create`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::create(Context &c) noexcept
@brief 在配置锁内执行网桥创建图；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5dbfe5bf884493ac"></a>

## `xv::net::filter::Engine::CommandFlow::delete_chain`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::delete_chain(Context &c) noexcept
@brief 调用规则核心删除空且无引用的用户链；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-69554bb5871cc51b"></a>

## `xv::net::filter::Engine::CommandFlow::flush`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::flush(Context &c) noexcept
@brief 清空规则、转发表或对应业务记录；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4d6886553411a1d8"></a>

## `xv::net::filter::Engine::CommandFlow::found`

```doxygen
@fn static inline bool xv::net::filter::Engine::CommandFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aa281b870971c1fa"></a>

## `xv::net::filter::Engine::CommandFlow::insert`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::insert(Context &c) noexcept
@brief 向重组槽或规则链插入已验证数据；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6f5f651c97674a7e"></a>

## `xv::net::filter::Engine::CommandFlow::inspect`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-89ffdeb9ea49342b"></a>

## `xv::net::filter::Engine::CommandFlow::invalid`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-25499c80890e1a05"></a>

## `xv::net::filter::Engine::CommandFlow::lookup`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::lookup(Context &c) noexcept
@brief 查找规则目标、记录或链内对象；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aaf4611bd7b006e3"></a>

## `xv::net::filter::Engine::CommandFlow::missing`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-333796d87ef84038"></a>

## `xv::net::filter::Engine::CommandFlow::policy`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::policy(Context &c) noexcept
@brief 设置或应用内建规则链的默认策略；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c517669ffe9497c0"></a>

## `xv::net::filter::Engine::CommandFlow::remove`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::remove(Context &c) noexcept
@brief 撤销选定网桥成员或删除规则记录；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-869eb132fbce0578"></a>

## `xv::net::filter::Engine::CommandFlow::rule`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::rule(Context &c) noexcept
@brief 序列化指定规则与逐条计数；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e564c94606a567cf"></a>

## `xv::net::filter::Engine::CommandFlow::valid`

```doxygen
@fn static inline bool xv::net::filter::Engine::CommandFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5cfed7eae5e37c38"></a>

## `xv::net::filter::Engine::CommandFlow::valid_name`

```doxygen
@fn static inline bool xv::net::filter::Engine::CommandFlow::valid_name(Context &c) noexcept
@brief 检查用户链名称是否符合规则；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-157128eef202c6c0"></a>

## `xv::net::filter::Engine::CommandFlow::valid_policy`

```doxygen
@fn static inline bool xv::net::filter::Engine::CommandFlow::valid_policy(Context &c) noexcept
@brief 检查内建链与 ACCEPT/DROP 策略约束；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-23eeca5fea8571b4"></a>

## `xv::net::filter::Engine::CommandFlow::zero`

```doxygen
@fn static inline void xv::net::filter::Engine::CommandFlow::zero(Context &c) noexcept
@brief 清零当前规则链与逐规则计数；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fca01367051ffb20"></a>

## `xv::net::filter::Engine::CommandFlow::operation`

```doxygen
@fn static bool xv::net::filter::Engine::CommandFlow::operation(Context &c) noexcept
@brief 检查命令编号是否等于模板指定编号；所属流程为 xv::net::filter::Engine::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9273219bb4415e84"></a>

## `xv::net::filter::Engine::EvaluationFlow::count`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::count(Context &c) noexcept
@brief 更新当前报文或重复确认的计数；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-376951f51cb8cf2e"></a>

## `xv::net::filter::Engine::EvaluationFlow::fetch`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::fetch(Context &c) noexcept
@brief 取出当前规则并推进链内规则游标；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-69d69941cfebeb05"></a>

## `xv::net::filter::Engine::EvaluationFlow::has_caller`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::has_caller(Context &c) noexcept
@brief 检查规则解释器是否还有显式用户链调用者；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c129d45f9ce7c827"></a>

## `xv::net::filter::Engine::EvaluationFlow::jump`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::jump(Context &c) noexcept
@brief 检查当前规则是否为用户链跳转；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-409d46f3ed0c31ce"></a>

## `xv::net::filter::Engine::EvaluationFlow::lookup`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::lookup(Context &c) noexcept
@brief 查找规则目标、记录或链内对象；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4222f8785a1bb6f4"></a>

## `xv::net::filter::Engine::EvaluationFlow::match`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::match(Context &c) noexcept
@brief 匹配本次报文与当前规则或候选对象；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-31ed373078ed9e41"></a>

## `xv::net::filter::Engine::EvaluationFlow::more_rules`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::more_rules(Context &c) noexcept
@brief 检查当前用户链是否还有规则；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7040950a9790dd73"></a>

## `xv::net::filter::Engine::EvaluationFlow::policy`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::policy(Context &c) noexcept
@brief 设置或应用内建规则链的默认策略；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0e28cb0897f52a5e"></a>

## `xv::net::filter::Engine::EvaluationFlow::pop`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::pop(Context &c) noexcept
@brief 恢复显式规则链返回栈中的调用者游标；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-63f232df0d8a8282"></a>

## `xv::net::filter::Engine::EvaluationFlow::push`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::push(Context &c) noexcept
@brief 保存用户链返回位置并进入目标链；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-68bcc8bfe72a417e"></a>

## `xv::net::filter::Engine::EvaluationFlow::root_valid`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::root_valid(Context &c) noexcept
@brief 检查规则解释器内建入口是否有效；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-34851c0233d7760f"></a>

## `xv::net::filter::Engine::EvaluationFlow::select_root`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::select_root(Context &c) noexcept
@brief 显式映射规则解释器的内建入口；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6031965f070aeda9"></a>

## `xv::net::filter::Engine::EvaluationFlow::terminal`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::terminal(Context &c) noexcept
@brief 检查当前规则是否直接给出 ACCEPT 或 DROP；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c40deed0388ed2ef"></a>

## `xv::net::filter::Engine::EvaluationFlow::valid_target`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::valid_target(Context &c) noexcept
@brief 检查规则跳转目标与显式返回栈容量；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d75e7924ac6bbb23"></a>

## `xv::net::filter::Engine::EvaluationFlow::verdict`

```doxygen
@fn static inline void xv::net::filter::Engine::EvaluationFlow::verdict(Context &c) noexcept
@brief 根据匹配规则生成接受或丢弃的业务判决；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6e8b3e3214b004f0"></a>

## `xv::net::filter::Engine::EvaluationFlow::within_budget`

```doxygen
@fn static inline bool xv::net::filter::Engine::EvaluationFlow::within_budget(Context &c) noexcept
@brief 检查并消耗本解释或扫描迭代的工作预算；所属流程为 xv::net::filter::Engine::EvaluationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 此预算按规则解释迭代计数，保持原有 1024 步含义。
```
