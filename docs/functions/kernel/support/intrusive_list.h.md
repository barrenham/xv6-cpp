# `kernel/support/intrusive_list.h`

源码：[打开实际文件](../../../../kernel/support/intrusive_list.h)。

接口、生命周期与验证见 [设计说明](../../../kernel-containers.md)。

## 函数导航

- [`xv::support::IntrusiveList::IntrusiveList<Node>`](intrusive_list.h.md#fn-0c7e514f52196856)
- [`xv::support::IntrusiveList::IntrusiveList<Node>`](intrusive_list.h.md#fn-0e3f6de031f9a9fd)
- [`xv::support::IntrusiveList::empty`](intrusive_list.h.md#fn-79ec00358ee612ff)
- [`xv::support::IntrusiveList::erase`](intrusive_list.h.md#fn-0accd4b7b84e4b5e)
- [`xv::support::IntrusiveList::first`](intrusive_list.h.md#fn-07560d425c1d7b2c)
- [`xv::support::IntrusiveList::last`](intrusive_list.h.md#fn-81d41161451c8e5d)
- [`xv::support::IntrusiveList::operator=`](intrusive_list.h.md#fn-d2a494e124287150)
- [`xv::support::IntrusiveList::pop_front`](intrusive_list.h.md#fn-55957948fb01881c)
- [`xv::support::IntrusiveList::push_back`](intrusive_list.h.md#fn-44a36888dd756964)

<a id="fn-0c7e514f52196856"></a>

## `xv::support::IntrusiveList::IntrusiveList<Node>`

```doxygen
@fn constexpr xv::support::IntrusiveList<Node>::IntrusiveList()
@brief 构造空的双向侵入式链表。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-0e3f6de031f9a9fd"></a>

## `xv::support::IntrusiveList::IntrusiveList<Node>`

```doxygen
@fn xv::support::IntrusiveList<Node>::IntrusiveList(const IntrusiveList &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-d2a494e124287150"></a>

## `xv::support::IntrusiveList::operator=`

```doxygen
@fn IntrusiveList &xv::support::IntrusiveList::operator=(const IntrusiveList &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-07560d425c1d7b2c"></a>

## `xv::support::IntrusiveList::first`

```doxygen
@fn Node *xv::support::IntrusiveList::first() const
@brief 借用首节点，空表返回空指针。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-81d41161451c8e5d"></a>

## `xv::support::IntrusiveList::last`

```doxygen
@fn Node *xv::support::IntrusiveList::last() const
@brief 借用尾节点，空表返回空指针。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-79ec00358ee612ff"></a>

## `xv::support::IntrusiveList::empty`

```doxygen
@fn bool xv::support::IntrusiveList::empty() const
@brief 检查逻辑 FIFO 是否为空。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-44a36888dd756964"></a>

## `xv::support::IntrusiveList::push_back`

```doxygen
@fn bool xv::support::IntrusiveList::push_back(Node &node)
@brief 将尚未入链的稳定节点追加到尾部；重复入链返回 false。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-0accd4b7b84e4b5e"></a>

## `xv::support::IntrusiveList::erase`

```doxygen
@fn void xv::support::IntrusiveList::erase(Node &node)
@brief 按节点身份移除链接；链表要求节点确实属于本表，哈希表对未知节点返回 false。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-55957948fb01881c"></a>

## `xv::support::IntrusiveList::pop_front`

```doxygen
@fn Node *xv::support::IntrusiveList::pop_front()
@brief 移除并返回首节点，空表返回空指针。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```
