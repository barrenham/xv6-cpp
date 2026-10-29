# `kernel/net/bridge/manager/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::BridgeManager::ExecuteFlow::activate`](flows.cppm.md#fn-36ea77630f8d053b)
- [`xv::net::BridgeManager::ExecuteFlow::administer`](flows.cppm.md#fn-3c0ef150a9ab78be)
- [`xv::net::BridgeManager::ExecuteFlow::already_active`](flows.cppm.md#fn-0a009be19890ef02)
- [`xv::net::BridgeManager::ExecuteFlow::busy`](flows.cppm.md#fn-576448f7f5a07967)
- [`xv::net::BridgeManager::ExecuteFlow::complete`](flows.cppm.md#fn-50bb860830371c08)
- [`xv::net::BridgeManager::ExecuteFlow::configure`](flows.cppm.md#fn-b449a8605697028d)
- [`xv::net::BridgeManager::ExecuteFlow::create`](flows.cppm.md#fn-4ee869055c249712)
- [`xv::net::BridgeManager::ExecuteFlow::deletable`](flows.cppm.md#fn-83c474481afa6645)
- [`xv::net::BridgeManager::ExecuteFlow::delete_bridge`](flows.cppm.md#fn-b3b416d5119f379f)
- [`xv::net::BridgeManager::ExecuteFlow::deleted`](flows.cppm.md#fn-b445d175ac3afb67)
- [`xv::net::BridgeManager::ExecuteFlow::exists`](flows.cppm.md#fn-292fda8e7ba652ca)
- [`xv::net::BridgeManager::ExecuteFlow::find_bridge`](flows.cppm.md#fn-061bdab99e4107b1)
- [`xv::net::BridgeManager::ExecuteFlow::find_interface`](flows.cppm.md#fn-1c672de2d10dea08)
- [`xv::net::BridgeManager::ExecuteFlow::find_name`](flows.cppm.md#fn-498b2d780120711d)
- [`xv::net::BridgeManager::ExecuteFlow::flush`](flows.cppm.md#fn-b5237372be5bae62)
- [`xv::net::BridgeManager::ExecuteFlow::found`](flows.cppm.md#fn-849658c4e7ee437d)
- [`xv::net::BridgeManager::ExecuteFlow::inspect`](flows.cppm.md#fn-2098cb22ee9e6955)
- [`xv::net::BridgeManager::ExecuteFlow::inspection`](flows.cppm.md#fn-714fd83d4ab751d9)
- [`xv::net::BridgeManager::ExecuteFlow::interface_found`](flows.cppm.md#fn-325b1ffd8e9b2222)
- [`xv::net::BridgeManager::ExecuteFlow::invalid`](flows.cppm.md#fn-eb53bd4fcb4af4e9)
- [`xv::net::BridgeManager::ExecuteFlow::invalidate`](flows.cppm.md#fn-a0c9f156fb1197af)
- [`xv::net::BridgeManager::ExecuteFlow::membership`](flows.cppm.md#fn-923b03552d74a8c8)
- [`xv::net::BridgeManager::ExecuteFlow::membership_operation`](flows.cppm.md#fn-d721a9f1407f3b10)
- [`xv::net::BridgeManager::ExecuteFlow::missing`](flows.cppm.md#fn-1e6442056b470624)
- [`xv::net::BridgeManager::ExecuteFlow::operation`](flows.cppm.md#fn-7b484caa5530a3d2)
- [`xv::net::BridgeManager::ExecuteFlow::publish`](flows.cppm.md#fn-09763140274daae0)
- [`xv::net::BridgeManager::ExecuteFlow::published`](flows.cppm.md#fn-9bc108c38006c743)
- [`xv::net::BridgeManager::ExecuteFlow::version`](flows.cppm.md#fn-65724960cc51c2e7)
- [`xv::net::BridgeManager::MembershipFlow::adding`](flows.cppm.md#fn-768e6c1cec772223)
- [`xv::net::BridgeManager::MembershipFlow::busy`](flows.cppm.md#fn-0b64d15bb519955f)
- [`xv::net::BridgeManager::MembershipFlow::commit`](flows.cppm.md#fn-be8eb4f735c44356)
- [`xv::net::BridgeManager::MembershipFlow::committed`](flows.cppm.md#fn-cb98b66709311787)
- [`xv::net::BridgeManager::MembershipFlow::disable_promiscuous`](flows.cppm.md#fn-b6e7a57216fce982)
- [`xv::net::BridgeManager::MembershipFlow::enable_promiscuous`](flows.cppm.md#fn-6cd86585d96edeea)
- [`xv::net::BridgeManager::MembershipFlow::find_empty`](flows.cppm.md#fn-703c873b93df2d48)
- [`xv::net::BridgeManager::MembershipFlow::find_member`](flows.cppm.md#fn-99d5eb6690abd4e1)
- [`xv::net::BridgeManager::MembershipFlow::full`](flows.cppm.md#fn-b0b09dbee828f994)
- [`xv::net::BridgeManager::MembershipFlow::inspect`](flows.cppm.md#fn-f0a8d2862f8d7b48)
- [`xv::net::BridgeManager::MembershipFlow::invalidate`](flows.cppm.md#fn-ec572e485f826b21)
- [`xv::net::BridgeManager::MembershipFlow::member`](flows.cppm.md#fn-c272ff42dca98c89)
- [`xv::net::BridgeManager::MembershipFlow::missing`](flows.cppm.md#fn-6bcb8a551fb182b2)
- [`xv::net::BridgeManager::MembershipFlow::prepare`](flows.cppm.md#fn-3f1e707bd50459fb)
- [`xv::net::BridgeManager::MembershipFlow::prepared`](flows.cppm.md#fn-e799fc90edfb382b)
- [`xv::net::BridgeManager::MembershipFlow::remove`](flows.cppm.md#fn-4a9ca77bb3b4812f)
- [`xv::net::BridgeManager::MembershipFlow::rollback`](flows.cppm.md#fn-9fd5b854d95874be)
- [`xv::net::BridgeManager::MembershipFlow::slot_available`](flows.cppm.md#fn-06780ce9188b8d16)
- [`xv::net::BridgeManager::MembershipFlow::supported`](flows.cppm.md#fn-a84b143e61f1cacc)
- [`xv::net::BridgeManager::MembershipFlow::unsupported`](flows.cppm.md#fn-d64b56a699e2ca13)
- [`xv::net::BridgeManager::MembershipFlow::unused`](flows.cppm.md#fn-8e9a8bc002b1446b)

<a id="fn-36ea77630f8d053b"></a>

## `xv::net::BridgeManager::ExecuteFlow::activate`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::activate(Context &c) noexcept
@brief 发布已成功登记的网桥活动状态；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3c0ef150a9ab78be"></a>

## `xv::net::BridgeManager::ExecuteFlow::administer`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::administer(Context &c) noexcept
@brief 在网桥管理锁下执行命令图；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 AdminGraph，其 stop 结束该次内部 run。
```

<a id="fn-0a009be19890ef02"></a>

## `xv::net::BridgeManager::ExecuteFlow::already_active`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::already_active(Context &c) noexcept
@brief 检查网桥是否已活动或已在接口表中；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-576448f7f5a07967"></a>

## `xv::net::BridgeManager::ExecuteFlow::busy`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-50bb860830371c08"></a>

## `xv::net::BridgeManager::ExecuteFlow::complete`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b449a8605697028d"></a>

## `xv::net::BridgeManager::ExecuteFlow::configure`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::configure(Context &c) noexcept
@brief 执行启动配置或受保护的配置命令图；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 ConfigurationGraph，其 stop 结束该次内部 run。
```

<a id="fn-4ee869055c249712"></a>

## `xv::net::BridgeManager::ExecuteFlow::create`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::create(Context &c) noexcept
@brief 在配置锁内执行网桥创建图；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 CreateGraph，其 stop 结束该次内部 run。
```

<a id="fn-83c474481afa6645"></a>

## `xv::net::BridgeManager::ExecuteFlow::deletable`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::deletable(Context &c) noexcept
@brief 检查网桥是否无地址路由和成员占用；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b3b416d5119f379f"></a>

## `xv::net::BridgeManager::ExecuteFlow::delete_bridge`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::delete_bridge(Context &c) noexcept
@brief 撤销网桥并清理转发表；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b445d175ac3afb67"></a>

## `xv::net::BridgeManager::ExecuteFlow::deleted`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::deleted(Context &c) noexcept
@brief 检查当前管理操作是否已经删除网桥；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-292fda8e7ba652ca"></a>

## `xv::net::BridgeManager::ExecuteFlow::exists`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::exists(Context &c) noexcept
@brief 记录同名或同路径对象已存在的错误；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-061bdab99e4107b1"></a>

## `xv::net::BridgeManager::ExecuteFlow::find_bridge`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::find_bridge(Context &c) noexcept
@brief 按请求接口标识查找活动网桥；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1c672de2d10dea08"></a>

## `xv::net::BridgeManager::ExecuteFlow::find_interface`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::find_interface(Context &c) noexcept
@brief 按管理请求选择接口对象；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-498b2d780120711d"></a>

## `xv::net::BridgeManager::ExecuteFlow::find_name`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::find_name(Context &c) noexcept
@brief 按请求名称选择预先构造的网桥对象；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b5237372be5bae62"></a>

## `xv::net::BridgeManager::ExecuteFlow::flush`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::flush(Context &c) noexcept
@brief 清空规则、转发表或对应业务记录；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-849658c4e7ee437d"></a>

## `xv::net::BridgeManager::ExecuteFlow::found`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2098cb22ee9e6955"></a>

## `xv::net::BridgeManager::ExecuteFlow::inspect`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-714fd83d4ab751d9"></a>

## `xv::net::BridgeManager::ExecuteFlow::inspection`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::inspection(Context &c) noexcept
@brief 检查本次管理命令是否为只读查询；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-325b1ffd8e9b2222"></a>

## `xv::net::BridgeManager::ExecuteFlow::interface_found`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::interface_found(Context &c) noexcept
@brief 检查管理命令的目标接口是否存在；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-eb53bd4fcb4af4e9"></a>

## `xv::net::BridgeManager::ExecuteFlow::invalid`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a0c9f156fb1197af"></a>

## `xv::net::BridgeManager::ExecuteFlow::invalidate`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-923b03552d74a8c8"></a>

## `xv::net::BridgeManager::ExecuteFlow::membership`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::membership(Context &c) noexcept
@brief 调用网桥成员变更事务；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d721a9f1407f3b10"></a>

## `xv::net::BridgeManager::ExecuteFlow::membership_operation`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::membership_operation(Context &c) noexcept
@brief 检查管理命令是否是成员增删；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e6442056b470624"></a>

## `xv::net::BridgeManager::ExecuteFlow::missing`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-09763140274daae0"></a>

## `xv::net::BridgeManager::ExecuteFlow::publish`

```doxygen
@fn static inline void xv::net::BridgeManager::ExecuteFlow::publish(Context &c) noexcept
@brief 封存并发布接口或调用接口注册表发布网桥；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9bc108c38006c743"></a>

## `xv::net::BridgeManager::ExecuteFlow::published`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::published(Context &c) noexcept
@brief 检查接口发布是否成功；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-65724960cc51c2e7"></a>

## `xv::net::BridgeManager::ExecuteFlow::version`

```doxygen
@fn static inline bool xv::net::BridgeManager::ExecuteFlow::version(Context &c) noexcept
@brief 检查管理请求的版本与固定结构大小；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7b484caa5530a3d2"></a>

## `xv::net::BridgeManager::ExecuteFlow::operation`

```doxygen
@fn static bool xv::net::BridgeManager::ExecuteFlow::operation(Context &c) noexcept
@brief 检查命令编号是否等于模板指定编号；所属流程为 xv::net::BridgeManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-768e6c1cec772223"></a>

## `xv::net::BridgeManager::MembershipFlow::adding`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::adding(Context &c) noexcept
@brief 检查本次是否是加入成员操作；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0b64d15bb519955f"></a>

## `xv::net::BridgeManager::MembershipFlow::busy`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-be8eb4f735c44356"></a>

## `xv::net::BridgeManager::MembershipFlow::commit`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-cb98b66709311787"></a>

## `xv::net::BridgeManager::MembershipFlow::committed`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::committed(Context &c) noexcept
@brief 检查成员变更的二次验证是否提交成功；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b6e7a57216fce982"></a>

## `xv::net::BridgeManager::MembershipFlow::disable_promiscuous`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::disable_promiscuous(Context &c) noexcept
@brief 撤销成员端口的混杂接收模式；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6cd86585d96edeea"></a>

## `xv::net::BridgeManager::MembershipFlow::enable_promiscuous`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::enable_promiscuous(Context &c) noexcept
@brief 请求成员端口启用混杂接收模式；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-703c873b93df2d48"></a>

## `xv::net::BridgeManager::MembershipFlow::find_empty`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::find_empty(Context &c) noexcept
@brief 扫描空闲网桥成员槽；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-99d5eb6690abd4e1"></a>

## `xv::net::BridgeManager::MembershipFlow::find_member`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::find_member(Context &c) noexcept
@brief 按端口对象身份查找已有网桥成员；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b0b09dbee828f994"></a>

## `xv::net::BridgeManager::MembershipFlow::full`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f0a8d2862f8d7b48"></a>

## `xv::net::BridgeManager::MembershipFlow::inspect`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ec572e485f826b21"></a>

## `xv::net::BridgeManager::MembershipFlow::invalidate`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c272ff42dca98c89"></a>

## `xv::net::BridgeManager::MembershipFlow::member`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::member(Context &c) noexcept
@brief 检查端口对象是否仍属于所选网桥；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6bcb8a551fb182b2"></a>

## `xv::net::BridgeManager::MembershipFlow::missing`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3f1e707bd50459fb"></a>

## `xv::net::BridgeManager::MembershipFlow::prepare`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::prepare(Context &c) noexcept
@brief 在配置锁内完成成员变更预检或撤销；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 PrepareGraph，其 stop 结束该次内部 run。
```

<a id="fn-e799fc90edfb382b"></a>

## `xv::net::BridgeManager::MembershipFlow::prepared`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::prepared(Context &c) noexcept
@brief 检查成员变更预检是否成功；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4a9ca77bb3b4812f"></a>

## `xv::net::BridgeManager::MembershipFlow::remove`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::remove(Context &c) noexcept
@brief 撤销选定网桥成员或删除规则记录；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9fd5b854d95874be"></a>

## `xv::net::BridgeManager::MembershipFlow::rollback`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::rollback(Context &c) noexcept
@brief 撤销未能提交的成员或租约变更；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-06780ce9188b8d16"></a>

## `xv::net::BridgeManager::MembershipFlow::slot_available`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::slot_available(Context &c) noexcept
@brief 检查网桥成员槽是否选择成功；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a84b143e61f1cacc"></a>

## `xv::net::BridgeManager::MembershipFlow::supported`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::supported(Context &c) noexcept
@brief 检查接口是否支持所请求的网络能力；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d64b56a699e2ca13"></a>

## `xv::net::BridgeManager::MembershipFlow::unsupported`

```doxygen
@fn static inline void xv::net::BridgeManager::MembershipFlow::unsupported(Context &c) noexcept
@brief 记录操作不支持的业务错误；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8e9a8bc002b1446b"></a>

## `xv::net::BridgeManager::MembershipFlow::unused`

```doxygen
@fn static inline bool xv::net::BridgeManager::MembershipFlow::unused(Context &c) noexcept
@brief 检查接口是否没有地址、DHCP 或路由占用；所属流程为 xv::net::BridgeManager::MembershipFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
