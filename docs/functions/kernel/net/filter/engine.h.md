# `kernel/net/filter/engine.h`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::filter::Counter::hit`](engine.h.md#fn-54c18d985ed82c9d)
- [`xv::net::filter::Engine::Engine`](engine.h.md#fn-1f8d4f4b636dbc78)
- [`xv::net::filter::Engine::accept`](engine.h.md#fn-d5fed1d71441705c)
- [`xv::net::filter::Engine::apply_policy`](engine.h.md#fn-946f84fdca33e969)
- [`xv::net::filter::Engine::create_chain`](engine.h.md#fn-1e3d52531133b5d8)
- [`xv::net::filter::Engine::execute`](engine.h.md#fn-286f6e66a00e3c34)
- [`xv::net::filter::Engine::find`](engine.h.md#fn-221e8216aeaa9254)
- [`xv::net::filter::Engine::insert_rule`](engine.h.md#fn-aedff2fc1780d51d)
- [`xv::net::filter::Engine::inspect_chain`](engine.h.md#fn-6a4ba3b3e89bd6ce)
- [`xv::net::filter::Engine::matches`](engine.h.md#fn-b9aab5ccd540a2b4)
- [`xv::net::filter::Engine::reaches`](engine.h.md#fn-bcf2e11e916d7b84)
- [`xv::net::filter::Engine::remove_chain`](engine.h.md#fn-1852ae2b61d2ea6d)
- [`xv::net::filter::Engine::remove_rule`](engine.h.md#fn-60da7da9fc9a98e8)
- [`xv::net::filter::Engine::valid`](engine.h.md#fn-ba4966ad176aeb45)
- [`xv::net::filter::equal`](engine.h.md#fn-dc6e0b233fe32320)
- [`xv::net::filter::mask`](engine.h.md#fn-f2bd1a7a86faf653)
- [`xv::net::filter::name_valid`](engine.h.md#fn-aab30a4b55c961b5)

<a id="fn-dc6e0b233fe32320"></a>

## `xv::net::filter::equal`

```doxygen
@fn inline bool xv::net::filter::equal(const char *left, const char *right)
@brief 在十六字节链名边界内比较终止字符串，拒绝未终止名称。
```

<a id="fn-aab30a4b55c961b5"></a>

## `xv::net::filter::name_valid`

```doxygen
@fn inline bool xv::net::filter::name_valid(const char *name)
@brief 检查链名非空、十五字符上限及允许的 ASCII 字符。
```

<a id="fn-f2bd1a7a86faf653"></a>

## `xv::net::filter::mask`

```doxygen
@fn inline unsigned xv::net::filter::mask(unsigned prefix)
@brief 为已验证的零至三十二位 IPv4 前缀生成地址掩码。
```

<a id="fn-54c18d985ed82c9d"></a>

## `xv::net::filter::Counter::hit`

```doxygen
@fn void xv::net::filter::Counter::hit(unsigned length)
@brief 累计一次匹配的包数与 IPv4 总字节数；调用方负责串行化。
```

<a id="fn-1f8d4f4b636dbc78"></a>

## `xv::net::filter::Engine::Engine`

```doxygen
@fn constexpr xv::net::filter::Engine::Engine()
@brief 构造空的 INPUT 与 OUTPUT 内建链，默认策略均为 ACCEPT。
```

<a id="fn-286f6e66a00e3c34"></a>

## `xv::net::filter::Engine::execute`

```doxygen
@fn inline int xv::net::filter::Engine::execute(unsigned operation, firewall_request &request)
@brief 校验管理 ABI，分派链查询、创建删除、策略修改及规则查询修改。
@return 查询成功 1、枚举结束或修改成功 0，失败返回负错误码。
@pre 调用方负责串行化。
@details 实现通过 CommandFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-d5fed1d71441705c"></a>

## `xv::net::filter::Engine::accept`

```doxygen
@fn inline bool xv::net::filter::Engine::accept(BuiltinChain entry_chain, const Metadata &packet)
@brief 从有类型的内建链入口顺序匹配规则，以显式返回栈执行自定义链跳转。
@details 显式 RETURN 与链尾共用返回路径；根链返回时统一应用策略并计数。每次判决最多执行 1024 步，目标无效、入口无效或超限均拒绝。 实现通过 EvaluationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用方串行化规则读写和计数；本函数不分配、不睡眠。
```

<a id="fn-221e8216aeaa9254"></a>

## `xv::net::filter::Engine::find`

```doxygen
@fn int xv::net::filter::Engine::find(const char *name) const
@brief 查找已占用链槽位，未找到返回负值。
```

<a id="fn-6a4ba3b3e89bd6ce"></a>

## `xv::net::filter::Engine::inspect_chain`

```doxygen
@fn int xv::net::filter::Engine::inspect_chain(firewall_request &request) const
@brief 按活动链的枚举次序查询名称、策略、规则数和默认策略计数；结束返回零。
@note 枚举次序不等同于内部槽位编号。
```

<a id="fn-1e3d52531133b5d8"></a>

## `xv::net::filter::Engine::create_chain`

```doxygen
@fn int xv::net::filter::Engine::create_chain(const char *name)
@brief 拒绝重复和保留名称，在空闲自定义链槽位中创建 RETURN 链；无容量返回 Full。
@pre 名称已通过 name_valid 校验。
```

<a id="fn-1852ae2b61d2ea6d"></a>

## `xv::net::filter::Engine::remove_chain`

```doxygen
@fn int xv::net::filter::Engine::remove_chain(unsigned owner)
@brief 只删除无规则且未被引用的自定义链，内建链或忙链返回 Busy。
```

<a id="fn-aedff2fc1780d51d"></a>

## `xv::net::filter::Engine::insert_rule`

```doxygen
@fn int xv::net::filter::Engine::insert_rule(unsigned owner, unsigned index, const firewall_rule &rule)
@brief 校验位置、规则和容量后移动后续条目，插入规则并清零新条目计数；失败保持原状态。
```

<a id="fn-60da7da9fc9a98e8"></a>

## `xv::net::filter::Engine::remove_rule`

```doxygen
@fn static int xv::net::filter::Engine::remove_rule(Chain &chain, unsigned index)
@brief 删除有效位置的规则并前移后续条目，保持顺序及剩余条目计数；越界返回 Missing。
```

<a id="fn-946f84fdca33e969"></a>

## `xv::net::filter::Engine::apply_policy`

```doxygen
@fn bool xv::net::filter::Engine::apply_policy(unsigned root, unsigned length)
@brief 累计内建入口链的默认策略计数，并根据该链策略返回判决。
```

<a id="fn-bcf2e11e916d7b84"></a>

## `xv::net::filter::Engine::reaches`

```doxygen
@fn bool xv::net::filter::Engine::reaches(unsigned from, unsigned to) const
@brief 用有界位图遍历跳转图，检查加入新边是否形成环路。
```

<a id="fn-ba4966ad176aeb45"></a>

## `xv::net::filter::Engine::valid`

```doxygen
@fn bool xv::net::filter::Engine::valid(const firewall_rule &rule, unsigned owner) const
@brief 验证规则的前缀、端口、接口方向、动作及跳转目标。
```

<a id="fn-b9aab5ccd540a2b4"></a>

## `xv::net::filter::Engine::matches`

```doxygen
@fn static bool xv::net::filter::Engine::matches(const firewall_rule &rule, const Metadata &packet)
@brief 对地址、协议、接口与可用端口执行条件合取匹配。
```
