# `kernel/support/ring_buffer.h`

源码：[打开实际文件](../../../../kernel/support/ring_buffer.h)。

接口、生命周期与验证见 [设计说明](../../../kernel-containers.md)。

## 函数导航

- [`xv::support::RingBuffer::capacity`](ring_buffer.h.md#fn-4348536d7a8ba9e8)
- [`xv::support::RingBuffer::clear`](ring_buffer.h.md#fn-bacd804e874636dd)
- [`xv::support::RingBuffer::empty`](ring_buffer.h.md#fn-a504bff2113aa4e5)
- [`xv::support::RingBuffer::front`](ring_buffer.h.md#fn-ce94895a69328275)
- [`xv::support::RingBuffer::full`](ring_buffer.h.md#fn-fd02a010c6a3fb6e)
- [`xv::support::RingBuffer::pop`](ring_buffer.h.md#fn-40164de8f1fc7b65)
- [`xv::support::RingBuffer::push`](ring_buffer.h.md#fn-0e1aa959399a87d8)
- [`xv::support::RingBuffer::size`](ring_buffer.h.md#fn-13751e2fbcc018ae)

<a id="fn-13751e2fbcc018ae"></a>

## `xv::support::RingBuffer::size`

```doxygen
@fn unsigned xv::support::RingBuffer::size() const
@brief 返回逻辑元素或位数。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-4348536d7a8ba9e8"></a>

## `xv::support::RingBuffer::capacity`

```doxygen
@fn constexpr unsigned xv::support::RingBuffer::capacity() const
@brief 返回编译期固定容量。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-a504bff2113aa4e5"></a>

## `xv::support::RingBuffer::empty`

```doxygen
@fn bool xv::support::RingBuffer::empty() const
@brief 检查逻辑 FIFO 是否为空。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-fd02a010c6a3fb6e"></a>

## `xv::support::RingBuffer::full`

```doxygen
@fn bool xv::support::RingBuffer::full() const
@brief 检查环形缓冲是否已满。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-0e1aa959399a87d8"></a>

## `xv::support::RingBuffer::push`

```doxygen
@fn bool xv::support::RingBuffer::push(const T &value)
@brief 仅在缓冲未满时写入并推进尾位置，不覆盖未读数据。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-40164de8f1fc7b65"></a>

## `xv::support::RingBuffer::pop`

```doxygen
@fn bool xv::support::RingBuffer::pop(T &value)
@brief 仅在缓冲非空时复制首值并推进读位置。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-ce94895a69328275"></a>

## `xv::support::RingBuffer::front`

```doxygen
@fn const T *xv::support::RingBuffer::front() const
@brief 借用首值地址，空缓冲返回空指针。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-bacd804e874636dd"></a>

## `xv::support::RingBuffer::clear`

```doxygen
@fn void xv::support::RingBuffer::clear()
@brief 清除逻辑位或重置环形缓冲的读写位置和数量。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```
