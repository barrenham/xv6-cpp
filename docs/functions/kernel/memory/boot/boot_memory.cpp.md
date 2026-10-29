# `kernel/memory/boot/boot_memory.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/boot/boot_memory.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:boot`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::FdtMemorySource::DiscoverFlow::begin`](boot_memory.cpp.md#fn-3126a42795505329)
- [`xv::memory::FdtMemorySource::DiscoverFlow::begin_token`](boot_memory.cpp.md#fn-a6e9dc633bec966e)
- [`xv::memory::FdtMemorySource::DiscoverFlow::blob`](boot_memory.cpp.md#fn-824af2c4f5317918)
- [`xv::memory::FdtMemorySource::DiscoverFlow::complete`](boot_memory.cpp.md#fn-97dde30630541980)
- [`xv::memory::FdtMemorySource::DiscoverFlow::end`](boot_memory.cpp.md#fn-eaa0dfe3c90e2040)
- [`xv::memory::FdtMemorySource::DiscoverFlow::end_token`](boot_memory.cpp.md#fn-98f9d5036babecd6)
- [`xv::memory::FdtMemorySource::DiscoverFlow::finish`](boot_memory.cpp.md#fn-51ecad9c6fbca662)
- [`xv::memory::FdtMemorySource::DiscoverFlow::finish_token`](boot_memory.cpp.md#fn-65be61e100b5ce80)
- [`xv::memory::FdtMemorySource::DiscoverFlow::nop_token`](boot_memory.cpp.md#fn-bbe9246b4df641bd)
- [`xv::memory::FdtMemorySource::DiscoverFlow::ok`](boot_memory.cpp.md#fn-880b02f674de3a49)
- [`xv::memory::FdtMemorySource::DiscoverFlow::property`](boot_memory.cpp.md#fn-feeb65253b8b242d)
- [`xv::memory::FdtMemorySource::DiscoverFlow::property_token`](boot_memory.cpp.md#fn-449a595049de81c7)
- [`xv::memory::FdtMemorySource::DiscoverFlow::publish`](boot_memory.cpp.md#fn-45b2b7602eb30c3c)
- [`xv::memory::FdtMemorySource::DiscoverFlow::read`](boot_memory.cpp.md#fn-1cbcf4798f93cedd)
- [`xv::memory::FdtMemorySource::DiscoverFlow::reservation`](boot_memory.cpp.md#fn-eac5b4951443007c)
- [`xv::memory::FdtMemorySource::DiscoverFlow::reservations_done`](boot_memory.cpp.md#fn-fde06a7fc4774555)
- [`xv::memory::FdtMemorySource::DiscoverFlow::start`](boot_memory.cpp.md#fn-e89fc30470ade6f0)
- [`xv::memory::FdtMemorySource::discover`](boot_memory.cpp.md#fn-9f01195f9922800f)
- [`xv::memory::RecordFlow::advance`](boot_memory.cpp.md#fn-a6d7935bedfdc97f)
- [`xv::memory::RecordFlow::ok`](boot_memory.cpp.md#fn-e93436aed3145e26)
- [`xv::memory::RecordFlow::publish`](boot_memory.cpp.md#fn-ffef0573f551aaa5)
- [`xv::memory::RecordFlow::remaining`](boot_memory.cpp.md#fn-e0d8f9beb3dda684)
- [`xv::memory::RecordFlow::success`](boot_memory.cpp.md#fn-3307003f7e2f9d41)
- [`xv::memory::RecordFlow::tuple`](boot_memory.cpp.md#fn-a148fb3d8b6a668d)
- [`xv::memory::RecordFlow::valid`](boot_memory.cpp.md#fn-f3f0ae8220a5e687)
- [`xv::memory::be32`](boot_memory.cpp.md#fn-dd45b9a05d70b534)
- [`xv::memory::be64`](boot_memory.cpp.md#fn-201a26e68ce57d1b)
- [`xv::memory::cells`](boot_memory.cpp.md#fn-353a9fac667d965e)
- [`xv::memory::cstring`](boot_memory.cpp.md#fn-4cc786925961e09d)
- [`xv::memory::fits`](boot_memory.cpp.md#fn-45133a6b64dfb030)
- [`xv::memory::node_name`](boot_memory.cpp.md#fn-5b3063eabf8ac76a)
- [`xv::memory::record`](boot_memory.cpp.md#fn-4f0358611c33fa8c)
- [`xv::memory::same`](boot_memory.cpp.md#fn-78b34bf73abd8715)

<a id="fn-dd45b9a05d70b534"></a>

## `xv::memory::be32`

```doxygen
@fn unsigned xv::memory::be32(const Byte *p)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取大端编码的 32 位值。
@param p 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`unsigned`。
@return 各分支实际返回：`(unsigned(p[0]) << 24) | (unsigned(p[1]) << 16) | (unsigned(p[2]) << 8) | unsigned(p[3])`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-201a26e68ce57d1b"></a>

## `xv::memory::be64`

```doxygen
@fn MemorySize xv::memory::be64(const Byte *p)
@brief 物理页所有权、地址范围、页表和用户内存复制：组合读取大端编码的 64 位值。
@param p 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`MemorySize` → `be32`。
@return 各分支实际返回：`(MemorySize(be32(p)) << 32) | be32(p + 4)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-78b34bf73abd8715"></a>

## `xv::memory::same`

```doxygen
@fn bool xv::memory::same(const char *left, const char *right)
@brief 物理页所有权、地址范围、页表和用户内存复制：比较设备树属性名或范围值是否相同。
@param left 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param right 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 需要特别跟踪的条件：`*left && *left == *right`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`*left == *right`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5b3063eabf8ac76a"></a>

## `xv::memory::node_name`

```doxygen
@fn bool xv::memory::node_name(const char *name, const char *wanted)
@brief 物理页所有权、地址范围、页表和用户内存复制：检查当前设备树节点的名称。
@param name 名称键或调试名称。
@param wanted 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 需要特别跟踪的条件：`*wanted && *name == *wanted`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`!*wanted && (!*name || *name == '@')`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-45133a6b64dfb030"></a>

## `xv::memory::fits`

```doxygen
@fn bool xv::memory::fits(MemorySize offset, MemorySize length, MemorySize total)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `offset <= total && length <= total - offset`，返回当前查询结果。
@param offset 本次操作使用的 偏移 值。
@param length 本次操作使用的 length 值。
@param total 本次操作使用的 total 值。
@return 各分支实际返回：`offset <= total && length <= total - offset`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4cc786925961e09d"></a>

## `xv::memory::cstring`

```doxygen
@fn bool xv::memory::cstring(const Byte *p, MemorySize available, MemorySize &length)
@brief 物理页所有权、地址范围、页表和用户内存复制：在设备树数据边界内读取零结尾字符串。
@param p 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param available 本次操作使用的 可用性 值。
@param length 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 需要特别跟踪的条件：`p[length] == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-353a9fac667d965e"></a>

## `xv::memory::cells`

```doxygen
@fn bool xv::memory::cells(const Byte *p, unsigned count, MemorySize &value)
@brief 物理页所有权、地址范围、页表和用户内存复制：按设备树地址或长度 cell 数组解码数值。
@param p 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@param value 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`be32` → `be64`。
@details 需要特别跟踪的条件：`count != 1 && count != 2`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4f0358611c33fa8c"></a>

## `xv::memory::record`

```doxygen
@fn bool xv::memory::record(const Node &node, BootMemoryInfo &info)
@brief 物理页所有权、地址范围、页表和用户内存复制：登记本次需要持久化的块或统计记录。
@param node inode 或链表节点对象。
@param info 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 RecordFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-9f01195f9922800f"></a>

## `xv::memory::FdtMemorySource::discover`

```doxygen
@fn bool xv::memory::FdtMemorySource::discover(BootMemoryInfo &result)
@brief 设备树内存信息读取器：从设备树或内存来源解析可用内存及保留区间。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DiscoverFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-3126a42795505329"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::begin`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::begin(Context &c) noexcept
@brief 验证节点名称和深度，继承父状态并压入固定栈。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a6e9dc633bec966e"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::begin_token`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::begin_token(Context &c) noexcept
@brief 检查 token 是否开始节点。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-824af2c4f5317918"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::blob`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::blob(Context &c) noexcept
@brief 仅在 DTB 实际位于 RAM 时验证其完整跨度并加入预留。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-97dde30630541980"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::complete`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::complete(Context &c) noexcept
@brief 检查结构已结束且已发现有效 RAM。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eaa0dfe3c90e2040"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::end`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::end(Context &c) noexcept
@brief 记录当前节点并弹栈，按原规则标记根关闭。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-98f9d5036babecd6"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::end_token`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::end_token(Context &c) noexcept
@brief 检查 token 是否结束节点。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-51ecad9c6fbca662"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::finish`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::finish(Context &c) noexcept
@brief 检查根已关闭且固定节点栈已清空。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-65be61e100b5ce80"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::finish_token`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::finish_token(Context &c) noexcept
@brief 检查 token 是否结束结构。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bbe9246b4df641bd"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::nop_token`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::nop_token(Context &c) noexcept
@brief 检查 token 是否为无操作。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-880b02f674de3a49"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::ok`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::ok(Context &c) noexcept
@brief 检查最近解析阶段是否成功。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-feeb65253b8b242d"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::property`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::property(Context &c) noexcept
@brief 验证属性边界与顺序并应用 cell、reg、状态及 no-map 字段。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-449a595049de81c7"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::property_token`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::property_token(Context &c) noexcept
@brief 检查 token 是否为属性。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-45b2b7602eb30c3c"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::publish`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::publish(Context &c) noexcept
@brief 将完整候选写回调用者输出并记录成功。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1cbcf4798f93cedd"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::read`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::read(Context &c) noexcept
@brief 在结构边界内读取下一 token 并推进游标。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eac5b4951443007c"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::reservation`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::reservation(Context &c) noexcept
@brief 验证并读取一对保留区地址长度，终止对不记录为范围。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fde06a7fc4774555"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::reservations_done`

```doxygen
@fn static bool xv::memory::FdtMemorySource::DiscoverFlow::reservations_done(Context &c) noexcept
@brief 检查硬件保留列表是否到达零终止对。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e89fc30470ade6f0"></a>

## `xv::memory::FdtMemorySource::DiscoverFlow::start`

```doxygen
@fn static void xv::memory::FdtMemorySource::DiscoverFlow::start(Context &c) noexcept
@brief 切换到结构块起点。
@details 头部验证仍先执行；局部 BootMemoryInfo 和固定 24 层 Node 栈由原函数持有，模板策略连接 token 分派与循环，全部解析和 blob 预留成功才写回输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a6d7935bedfdc97f"></a>

## `xv::memory::RecordFlow::advance`

```doxygen
@fn static void xv::memory::RecordFlow::advance(Context &c) noexcept
@brief 推进到下一个 reg 元组。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e93436aed3145e26"></a>

## `xv::memory::RecordFlow::ok`

```doxygen
@fn static bool xv::memory::RecordFlow::ok(Context &c) noexcept
@brief 检查最近元组解码或记录是否成功。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ffef0573f551aaa5"></a>

## `xv::memory::RecordFlow::publish`

```doxygen
@fn static void xv::memory::RecordFlow::publish(Context &c) noexcept
@brief 将当前元组按节点类别记录为 RAM 或预留。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e0d8f9beb3dda684"></a>

## `xv::memory::RecordFlow::remaining`

```doxygen
@fn static bool xv::memory::RecordFlow::remaining(Context &c) noexcept
@brief 检查 reg 属性是否仍有待解码元组。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3307003f7e2f9d41"></a>

## `xv::memory::RecordFlow::success`

```doxygen
@fn static void xv::memory::RecordFlow::success(Context &c) noexcept
@brief 记录整个节点 reg 解析成功。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a148fb3d8b6a668d"></a>

## `xv::memory::RecordFlow::tuple`

```doxygen
@fn static void xv::memory::RecordFlow::tuple(Context &c) noexcept
@brief 按大端 cell 解码地址长度并验证非零和溢出。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f3f0ae8220a5e687"></a>

## `xv::memory::RecordFlow::valid`

```doxygen
@fn static bool xv::memory::RecordFlow::valid(Context &c) noexcept
@brief 检查 reg 存在、元组完整且地址与长度 cell 数受支持。
@details 无关或禁用节点在入口直接成功；验证 reg 的 cell 格式和完整元组，逐元组检查溢出，再记录 RAM 或预留，失败只影响上层局部候选。 下一节点由图策略声明，业务结果与控制信号分离。
```
