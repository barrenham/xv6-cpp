# `kernel/memory/address/physical_address.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/address/physical_address.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:address`。

## 函数导航

- [`xv::memory::PhysicalAddress::PhysicalAddress`](physical_address.cppm.md#fn-c7f2f36c782bd86d)
- [`xv::memory::PhysicalAddress::PhysicalAddress`](physical_address.cppm.md#fn-5e8046f311ca6ec6)
- [`xv::memory::PhysicalAddress::advanced`](physical_address.cppm.md#fn-6b709c8151e3b178)
- [`xv::memory::PhysicalAddress::align_down`](physical_address.cppm.md#fn-0a7751e8fc838cf8)
- [`xv::memory::PhysicalAddress::align_up`](physical_address.cppm.md#fn-dacc0eb41d4fe37e)
- [`xv::memory::PhysicalAddress::from_pointer`](physical_address.cppm.md#fn-5766400bb534679b)
- [`xv::memory::PhysicalAddress::is_aligned`](physical_address.cppm.md#fn-001f6ccdad64cdda)
- [`xv::memory::PhysicalAddress::is_null`](physical_address.cppm.md#fn-8b823163a1404ec3)
- [`xv::memory::PhysicalAddress::offset_from`](physical_address.cppm.md#fn-35eac07f2caaac23)
- [`xv::memory::PhysicalAddress::page_number`](physical_address.cppm.md#fn-36c923fca9818fab)
- [`xv::memory::PhysicalAddress::pointer`](physical_address.cppm.md#fn-0fefe5f12cb79cfc)
- [`xv::memory::PhysicalAddress::raw`](physical_address.cppm.md#fn-10b6a760d8c57bac)
- [`xv::memory::detail::fail`](../boot/kalloc.cpp.md#fn-b1d0993dea3ca642)
- [`xv::memory::operator<`](physical_address.cppm.md#fn-a5a61a515a6923c9)
- [`xv::memory::operator<=`](physical_address.cppm.md#fn-75ff597b76e1d317)
- [`xv::memory::operator==`](physical_address.cppm.md#fn-2d9efc55e17c595a)
- [`xv::memory::operator>`](physical_address.cppm.md#fn-7a63245f682c3269)
- [`xv::memory::operator>=`](physical_address.cppm.md#fn-d2c291c5a56b4ada)

<a id="fn-c7f2f36c782bd86d"></a>

## `xv::memory::PhysicalAddress::PhysicalAddress`

```doxygen
@fn constexpr xv::memory::PhysicalAddress::PhysicalAddress()
@brief 构造物理地址值对象，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5e8046f311ca6ec6"></a>

## `xv::memory::PhysicalAddress::PhysicalAddress`

```doxygen
@fn explicit constexpr xv::memory::PhysicalAddress::PhysicalAddress(Value value)
@brief 构造物理地址值对象，建立本重载要求的成员初值和借用关系。
@param value 本次操作使用的 value 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`value_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5766400bb534679b"></a>

## `xv::memory::PhysicalAddress::from_pointer`

```doxygen
@fn static PhysicalAddress xv::memory::PhysicalAddress::from_pointer(const void *pointer)
@brief 物理地址值对象：读取或计算 `PhysicalAddress(reinterpret_cast<Value>(pointer))`，返回当前查询结果。
@param pointer 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress`。
@return 各分支实际返回：`PhysicalAddress(reinterpret_cast<Value>(pointer))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-10b6a760d8c57bac"></a>

## `xv::memory::PhysicalAddress::raw`

```doxygen
@fn constexpr Value xv::memory::PhysicalAddress::raw() const
@brief 物理地址值对象：读取或计算 `value_`，返回当前查询结果。
@return 各分支实际返回：`value_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0fefe5f12cb79cfc"></a>

## `xv::memory::PhysicalAddress::pointer`

```doxygen
@fn T *xv::memory::PhysicalAddress::pointer() const
@brief 物理地址值对象：读取或计算 `reinterpret_cast<T *>(value_)`，返回当前查询结果。
@return 各分支实际返回：`reinterpret_cast<T *>(value_)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8b823163a1404ec3"></a>

## `xv::memory::PhysicalAddress::is_null`

```doxygen
@fn constexpr bool xv::memory::PhysicalAddress::is_null() const
@brief 物理地址值对象：读取或计算 `value_ == 0`，返回当前查询结果。
@return 各分支实际返回：`value_ == 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-001f6ccdad64cdda"></a>

## `xv::memory::PhysicalAddress::is_aligned`

```doxygen
@fn constexpr bool xv::memory::PhysicalAddress::is_aligned(MemorySize alignment = PageSize) const
@brief 物理地址值对象：读取或计算 `alignment != 0 && value_ % alignment == 0`，返回当前查询结果。
@param alignment 本次操作使用的 alignment 值。
@return 各分支实际返回：`alignment != 0 && value_ % alignment == 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0a7751e8fc838cf8"></a>

## `xv::memory::PhysicalAddress::align_down`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalAddress::align_down(MemorySize alignment = PageSize) const
@brief 物理地址值对象：读取或计算 `PhysicalAddress(value_ - value_ % alignment)`，返回当前查询结果。
@param alignment 本次操作使用的 alignment 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `PhysicalAddress`。
@details 需要特别跟踪的条件：`alignment == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`PhysicalAddress(value_ - value_ % alignment)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-dacc0eb41d4fe37e"></a>

## `xv::memory::PhysicalAddress::align_up`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalAddress::align_up(MemorySize alignment = PageSize) const
@brief 物理地址值对象：对齐向上。
@param alignment 本次操作使用的 alignment 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `advanced`。
@details 需要特别跟踪的条件：`alignment == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`remainder == 0 ? *this : advanced(alignment - remainder)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6b709c8151e3b178"></a>

## `xv::memory::PhysicalAddress::advanced`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalAddress::advanced(MemorySize bytes) const
@brief 物理地址值对象：读取或计算 `PhysicalAddress(value_ + bytes)`，返回当前查询结果。
@param bytes 本次操作使用的 字节区域 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Value` → `detail::fail` → `PhysicalAddress`。
@details 需要特别跟踪的条件：`bytes > ~Value(0) - value_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`PhysicalAddress(value_ + bytes)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-35eac07f2caaac23"></a>

## `xv::memory::PhysicalAddress::offset_from`

```doxygen
@fn constexpr MemorySize xv::memory::PhysicalAddress::offset_from(PhysicalAddress origin) const
@brief 物理地址值对象：读取或计算 `value_ - origin.value_`，返回当前查询结果。
@param origin 本次操作使用的 origin 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail`。
@details 需要特别跟踪的条件：`*this < origin`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`value_ - origin.value_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-36c923fca9818fab"></a>

## `xv::memory::PhysicalAddress::page_number`

```doxygen
@fn constexpr PageIndex xv::memory::PhysicalAddress::page_number() const
@brief 物理地址值对象：读取或计算 `value_ / PageSize`，返回当前查询结果。
@return 各分支实际返回：`value_ / PageSize`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2d9efc55e17c595a"></a>

## `xv::memory::operator==`

```doxygen
@fn friend constexpr bool xv::memory::operator==(PhysicalAddress, PhysicalAddress)
@brief 按 == 运算符比较或变换物理地址值对象的底层值。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 返回类型为 `bool (PhysicalAddress, PhysicalAddress)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-a5a61a515a6923c9"></a>

## `xv::memory::operator<`

```doxygen
@fn friend constexpr bool xv::memory::operator<(PhysicalAddress left, PhysicalAddress right)
@brief 按 < 运算符比较或变换物理地址值对象的底层值。
@param left 本次操作使用的 left 值。
@param right 本次操作使用的 right 值。
@return 各分支实际返回：`left.value_ < right.value_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-75ff597b76e1d317"></a>

## `xv::memory::operator<=`

```doxygen
@fn friend constexpr bool xv::memory::operator<=(PhysicalAddress left, PhysicalAddress right)
@brief 按 <= 运算符比较或变换物理地址值对象的底层值。
@param left 本次操作使用的 left 值。
@param right 本次操作使用的 right 值。
@return 各分支实际返回：`left.value_ <= right.value_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7a63245f682c3269"></a>

## `xv::memory::operator>`

```doxygen
@fn friend constexpr bool xv::memory::operator>(PhysicalAddress left, PhysicalAddress right)
@brief 按 > 运算符比较或变换物理地址值对象的底层值。
@param left 本次操作使用的 left 值。
@param right 本次操作使用的 right 值。
@return 各分支实际返回：`right < left`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d2c291c5a56b4ada"></a>

## `xv::memory::operator>=`

```doxygen
@fn friend constexpr bool xv::memory::operator>=(PhysicalAddress left, PhysicalAddress right)
@brief 按 >= 运算符比较或变换物理地址值对象的底层值。
@param left 本次操作使用的 left 值。
@param right 本次操作使用的 right 值。
@return 各分支实际返回：`right <= left`。应结合上面的条件检查区分正常值、空值和错误值。
```
