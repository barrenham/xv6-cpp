# `kernel/support/bitmap.h`

源码：[打开实际文件](../../../../kernel/support/bitmap.h)。

接口、生命周期与验证见 [设计说明](../../../kernel-containers.md)。

## 函数导航

- [`xv::support::BitmapView::BitmapView`](bitmap.h.md#fn-e18dfebc00a3eda4)
- [`xv::support::BitmapView::clear`](bitmap.h.md#fn-5e9267dc3e2f76f6)
- [`xv::support::BitmapView::first_clear`](bitmap.h.md#fn-764d063ce0c18f38)
- [`xv::support::BitmapView::set`](bitmap.h.md#fn-1f2fd2b7b97ee9d4)
- [`xv::support::BitmapView::size`](bitmap.h.md#fn-c4acabb4c0b04d0b)
- [`xv::support::BitmapView::test`](bitmap.h.md#fn-2806b9af9cc01109)

<a id="fn-e18dfebc00a3eda4"></a>

## `xv::support::BitmapView::BitmapView`

```doxygen
@fn constexpr xv::support::BitmapView::BitmapView(unsigned long *words, unsigned long bits)
@brief 构造外部存储上的位图视图，不分配或清零存储。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-c4acabb4c0b04d0b"></a>

## `xv::support::BitmapView::size`

```doxygen
@fn unsigned long xv::support::BitmapView::size() const
@brief 返回逻辑元素或位数。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-2806b9af9cc01109"></a>

## `xv::support::BitmapView::test`

```doxygen
@fn bool xv::support::BitmapView::test(unsigned long bit) const
@brief 在逻辑边界内查询一位，越界返回 false。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-1f2fd2b7b97ee9d4"></a>

## `xv::support::BitmapView::set`

```doxygen
@fn bool xv::support::BitmapView::set(unsigned long bit)
@brief 在逻辑边界内设置一位，越界不修改存储。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-5e9267dc3e2f76f6"></a>

## `xv::support::BitmapView::clear`

```doxygen
@fn bool xv::support::BitmapView::clear(unsigned long bit)
@brief 清除逻辑位或重置环形缓冲的读写位置和数量。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```

<a id="fn-764d063ce0c18f38"></a>

## `xv::support::BitmapView::first_clear`

```doxygen
@fn unsigned long xv::support::BitmapView::first_clear(unsigned long start = 0) const
@brief 从给定位置扫描非满字并用 ctz 选择空位，不返回末字填充位；未找到返回逻辑位数。
@details 调用方管理稳定节点或底层存储及并发排他；容器不取得锁、不接管节点内存。
```
