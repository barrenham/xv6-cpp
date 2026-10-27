# `kernel/fs/buffer/page_buffer.cpp`

## 函数导航

- [`xv::fs::PageBuffer::append`](page_buffer.cpp.md#fn-c5618dcd4420a2eb)
- [`xv::fs::PageBuffer::clear`](page_buffer.cpp.md#fn-882e25e0a5ff62b9)
- [`xv::fs::PageBuffer::page`](page_buffer.cpp.md#fn-7324b6e1a8cb6d8b)
- [`xv::fs::PageBuffer::page`](page_buffer.cpp.md#fn-afb49e59fd42539a)
- [`xv::fs::PageBuffer::read`](page_buffer.cpp.md#fn-737bf8707fc7f001)
- [`xv::fs::PageBuffer::take`](page_buffer.cpp.md#fn-a9cb29f61972a3dd)
- [`xv::fs::PageBuffer::write`](page_buffer.cpp.md#fn-f0d6953988455923)
- [`xv::fs::PageBuffer::~PageBuffer`](page_buffer.cpp.md#fn-522ff50df4a42b80)

<a id="fn-522ff50df4a42b80"></a>

## `xv::fs::PageBuffer::~PageBuffer`

```doxygen
@fn xv::fs::PageBuffer::~PageBuffer()
@brief 归还缓冲拥有的全部物理页。
@details 委托 clear；覆盖普通关闭、快照生成失败及部分初始化后的回滚。
```

<a id="fn-a9cb29f61972a3dd"></a>

## `xv::fs::PageBuffer::take`

```doxygen
@fn void xv::fs::PageBuffer::take(PageBuffer &other)
@brief 转移另一缓冲的页链并清空来源。
@details 先清理当前已有页，再接管首尾指针、页数和长度；来源成为可析构的空缓冲，自转移不改变状态。调用者串行访问两个对象。
```

<a id="fn-afb49e59fd42539a"></a>

## `xv::fs::PageBuffer::page`

```doxygen
@fn PageBuffer::Page *xv::fs::PageBuffer::page(unsigned index, bool create)
@brief 按页下标定位数据块，并按重载契约决定是否分配。
@details 只读重载仅查询已有链；可写重载在 create 为真时逐页申请并零初始化，分配失败返回 nullptr。尾页有直接定位路径，其余从链首遍历；对象本身不加锁，所属节点或快照拥有者负责串行化。
```

<a id="fn-7324b6e1a8cb6d8b"></a>

## `xv::fs::PageBuffer::page`

```doxygen
@fn const PageBuffer::Page *xv::fs::PageBuffer::page(unsigned index) const
@brief 按页下标定位数据块，并按重载契约决定是否分配。
@details 只读重载仅查询已有链；可写重载在 create 为真时逐页申请并零初始化，分配失败返回 nullptr。尾页有直接定位路径，其余从链首遍历；对象本身不加锁，所属节点或快照拥有者负责串行化。
```

<a id="fn-c5618dcd4420a2eb"></a>

## `xv::fs::PageBuffer::append`

```doxygen
@fn bool xv::fs::PageBuffer::append(const void *source, unsigned count)
@brief 把内核缓冲追加到逻辑末尾。
@details 委托 write，拒绝超过有符号长度范围的请求，只有完整复制才返回 true。部分失败可能保留前缀；快照生成方须把失败视为整份快照失败并销毁缓冲。
```

<a id="fn-f0d6953988455923"></a>

## `xv::fs::PageBuffer::write`

```doxygen
@fn int xv::fs::PageBuffer::write(unsigned offset, bool user, unsigned long source, unsigned count)
@brief 按字节偏移复制用户或内核数据，并按需增加页面。
@details 检查偏移与长度溢出，越过 EOF 的间隙显式清零。逐页分配和复制，成功部分才推进逻辑长度；失败返回已写字节数或 -1，零长度不访问源地址。已申请但尚无逻辑数据的页仍由本缓冲拥有并在 clear 回收。
```

<a id="fn-737bf8707fc7f001"></a>

## `xv::fs::PageBuffer::read`

```doxygen
@fn int xv::fs::PageBuffer::read(unsigned offset, bool user, unsigned long destination, unsigned count) const
@brief 从页链复制指定字节范围。
@details 把读取裁剪到逻辑 EOF，逐页 copy_out 并区分用户和内核地址空间。复制失败返回先前已完成的字节数或 -1；本函数不维护文件偏移，由上层根据返回值推进。
```

<a id="fn-882e25e0a5ff62b9"></a>

## `xv::fs::PageBuffer::clear`

```doxygen
@fn void xv::fs::PageBuffer::clear()
@brief 逐页回收所有内容并恢复空缓冲状态。
@details 遍历页链，在释放当前页前保存 next，最终清空首尾、页数与长度。不会保留旧内容供后续读取，调用方保证没有并发使用该缓冲。
```
