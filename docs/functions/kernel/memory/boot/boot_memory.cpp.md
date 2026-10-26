# `kernel/memory/boot/boot_memory.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/boot/boot_memory.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:boot`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::FdtMemorySource::discover`](boot_memory.cpp.md#fn-9f01195f9922800f)
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cells` → `MemorySize` → `range` → `PhysicalAddress` → `info.set_ram` → `info.reserve`。
@details 需要特别跟踪的条件：`node.disabled || (!node.memory && !node.reserved)`；`!node.reg || !stride || node.reg_size % stride != 0 || node.parent_address < 1 || node.parent_address > 2 || node.parent_size < 1 || node.parent_size > 2`；`node.memory ? !info.set_ram(range) : !info.reserve(range, node.no_map)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9f01195f9922800f"></a>

## `xv::memory::FdtMemorySource::discover`

```doxygen
@fn bool xv::memory::FdtMemorySource::discover(BootMemoryInfo &result)
@brief 设备树内存信息读取器：从设备树或内存来源解析可用内存及保留区间。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`be32` → `fits` → `be64` → `MemorySize` → `found.reserve` → `PhysicalMemoryRange` → `PhysicalAddress` → `cstring` → `node_name` → `same` → `record` → `found.ram`。
@details 本函数可见的状态更新包括：`nodes[depth - 1].children_started = true;`；`node.parent_address = nodes[depth - 1].child_address;`；`node.parent_size = nodes[depth - 1].child_size;`；`node.reserved = nodes[depth - 1].reserved_container;`；`node.disabled = nodes[depth - 1].disabled;`；`node.memory = depth == 1 && node_name(name, "memory");`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!blob_ || capacity_ < 40 || be32(blob_) != 0xd00dfeed`；`!fits(cursor, 16, total) || (cursor < structure + structure_size && cursor + 16 > structure) || (cursor < strings + string_size && cursor + 16 > strings)`；`!address && !size`；`size > ~MemorySize(0) - address || !found.reserve(PhysicalMemoryRange(PhysicalAddress(address), PhysicalAddress(address + size)))`；`fits(cursor, 4, stop)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```
