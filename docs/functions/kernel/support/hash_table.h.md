# `kernel/support/hash_table.h`

源码：[打开实际文件](../../../../kernel/support/hash_table.h)。

接口、生命周期与验证见 [设计说明](../../../kernel-containers.md)。

## 函数导航

- [`xv::support::IntrusiveHashTable::IntrusiveHashTable<Node, Key, Traits, Buckets>`](hash_table.h.md#fn-ca7e14c2146a1a63)
- [`xv::support::IntrusiveHashTable::IntrusiveHashTable<Node, Key, Traits, Buckets>`](hash_table.h.md#fn-fea41f17ec1904d0)
- [`xv::support::IntrusiveHashTable::erase`](hash_table.h.md#fn-6a0ee2dcd30ca0c9)
- [`xv::support::IntrusiveHashTable::find`](hash_table.h.md#fn-26ab0eade502a7a2)
- [`xv::support::IntrusiveHashTable::index`](hash_table.h.md#fn-98431d5bcaa15f6f)
- [`xv::support::IntrusiveHashTable::insert`](hash_table.h.md#fn-896cbb6422b54ad7)
- [`xv::support::IntrusiveHashTable::operator=`](hash_table.h.md#fn-656384e3a714600e)
- [`xv::support::IntrusiveHashTable::size`](hash_table.h.md#fn-8bba0f764b8698f1)
- [`xv::support::hash_bytes`](hash_table.h.md#fn-552010898fa41d23)
- [`xv::support::hash_integer`](hash_table.h.md#fn-fad1d804ce749f5d)

<a id="fn-fad1d804ce749f5d"></a>

## `xv::support::hash_integer`

```doxygen
@fn constexpr unsigned long xv::support::hash_integer(unsigned long value)
@brief 对 64 位整数进行非密码学混合。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-552010898fa41d23"></a>

## `xv::support::hash_bytes`

```doxygen
@fn inline unsigned long xv::support::hash_bytes(const void *bytes, unsigned long size)
@brief 按给定字节长度计算 FNV-1a 散列，不依赖零终止。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-fea41f17ec1904d0"></a>

## `xv::support::IntrusiveHashTable::IntrusiveHashTable<Node, Key, Traits, Buckets>`

```doxygen
@fn constexpr xv::support::IntrusiveHashTable<Node, Key, Traits, Buckets>::IntrusiveHashTable()
@brief 构造空的固定桶哈希索引。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-ca7e14c2146a1a63"></a>

## `xv::support::IntrusiveHashTable::IntrusiveHashTable<Node, Key, Traits, Buckets>`

```doxygen
@fn xv::support::IntrusiveHashTable<Node, Key, Traits, Buckets>::IntrusiveHashTable(const IntrusiveHashTable &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-656384e3a714600e"></a>

## `xv::support::IntrusiveHashTable::operator=`

```doxygen
@fn IntrusiveHashTable &xv::support::IntrusiveHashTable::operator=(const IntrusiveHashTable &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-8bba0f764b8698f1"></a>

## `xv::support::IntrusiveHashTable::size`

```doxygen
@fn unsigned long xv::support::IntrusiveHashTable::size() const
@brief 返回逻辑元素或位数。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-26ab0eade502a7a2"></a>

## `xv::support::IntrusiveHashTable::find`

```doxygen
@fn Node *xv::support::IntrusiveHashTable::find(const Key &key) const
@brief 沿对应桶的碰撞链查找相等键，返回借用节点或空指针。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-896cbb6422b54ad7"></a>

## `xv::support::IntrusiveHashTable::insert`

```doxygen
@fn bool xv::support::IntrusiveHashTable::insert(Node &node)
@brief 拒绝重复节点和重复键后发布到桶链，不分配内存。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-6a0ee2dcd30ca0c9"></a>

## `xv::support::IntrusiveHashTable::erase`

```doxygen
@fn bool xv::support::IntrusiveHashTable::erase(Node &node)
@brief 按节点身份移除链接；链表要求节点确实属于本表，哈希表对未知节点返回 false。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-98431d5bcaa15f6f"></a>

## `xv::support::IntrusiveHashTable::index`

```doxygen
@fn static unsigned xv::support::IntrusiveHashTable::index(const Key &key)
@brief 通过 Traits 散列与二次幂桶掩码计算桶号。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```
