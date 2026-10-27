# `kernel/fs/buffer/page_buffer.cppm`

## 函数导航

- [`xv::fs::PageBuffer::PageBuffer`](page_buffer.cppm.md#fn-dee2de49cc9fa318)
- [`xv::fs::PageBuffer::PageBuffer`](page_buffer.cppm.md#fn-27ff943701896e5d)
- [`xv::fs::PageBuffer::append`](page_buffer.cpp.md#fn-c5618dcd4420a2eb)
- [`xv::fs::PageBuffer::clear`](page_buffer.cpp.md#fn-882e25e0a5ff62b9)
- [`xv::fs::PageBuffer::operator=`](page_buffer.cppm.md#fn-1147eade9549dc0c)
- [`xv::fs::PageBuffer::page`](page_buffer.cpp.md#fn-7324b6e1a8cb6d8b)
- [`xv::fs::PageBuffer::page`](page_buffer.cpp.md#fn-afb49e59fd42539a)
- [`xv::fs::PageBuffer::read`](page_buffer.cpp.md#fn-737bf8707fc7f001)
- [`xv::fs::PageBuffer::size`](page_buffer.cppm.md#fn-a1b9d61afac265b6)
- [`xv::fs::PageBuffer::take`](page_buffer.cpp.md#fn-a9cb29f61972a3dd)
- [`xv::fs::PageBuffer::write`](page_buffer.cpp.md#fn-f0d6953988455923)
- [`xv::fs::PageBuffer::~PageBuffer`](page_buffer.cpp.md#fn-522ff50df4a42b80)

<a id="fn-dee2de49cc9fa318"></a>

## `xv::fs::PageBuffer::PageBuffer`

```doxygen
@fn constexpr xv::fs::PageBuffer::PageBuffer() = default
@brief 构造空的按页增长缓冲。
@details 默认状态不申请物理页。显式禁用复制，避免重复拥有同一页链；take 用于转移拥有权。
```

<a id="fn-27ff943701896e5d"></a>

## `xv::fs::PageBuffer::PageBuffer`

```doxygen
@fn xv::fs::PageBuffer::PageBuffer(const PageBuffer &) = delete
@brief 构造空的按页增长缓冲。
@details 默认状态不申请物理页。显式禁用复制，避免重复拥有同一页链；take 用于转移拥有权。
```

<a id="fn-1147eade9549dc0c"></a>

## `xv::fs::PageBuffer::operator=`

```doxygen
@fn PageBuffer &xv::fs::PageBuffer::operator=(const PageBuffer &) = delete
@brief 在编译期禁止复制缓冲拥有权。
@details 该操作被删除；需要转移资源时使用 take，不能让两个对象负责释放同一页链。
```

<a id="fn-a1b9d61afac265b6"></a>

## `xv::fs::PageBuffer::size`

```doxygen
@fn unsigned xv::fs::PageBuffer::size() const
@brief 读取缓冲的逻辑字节长度。
@details 长度可跨越任意多个页，不等同于分配容量。调用者按所属节点锁或打开快照的不变性保证同步。
```
