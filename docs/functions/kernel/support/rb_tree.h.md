# `kernel/support/rb_tree.h`

源码：[打开实际文件](../../../../kernel/support/rb_tree.h)。

该文件的红黑树或地址池职责见 [内核虚拟地址池](../../../virtual-address-pool.md)。

## 函数导航

- [`xv::support::RbTree::RbTree<Node, Compare>`](rb_tree.h.md#fn-b2d2d3c83e01b68e)
- [`xv::support::RbTree::RbTree<Node, Compare>`](rb_tree.h.md#fn-bbe9b658eca338ad)
- [`xv::support::RbTree::erase`](rb_tree.h.md#fn-6bded9bb25d0e1e1)
- [`xv::support::RbTree::erase_fix`](rb_tree.h.md#fn-936f3065732be42f)
- [`xv::support::RbTree::find`](rb_tree.h.md#fn-9e274d7f038a163f)
- [`xv::support::RbTree::first`](rb_tree.h.md#fn-d72d79319fd7ec4c)
- [`xv::support::RbTree::insert`](rb_tree.h.md#fn-db5911cce7ad202a)
- [`xv::support::RbTree::insert_fix`](rb_tree.h.md#fn-a560d2326a30a61d)
- [`xv::support::RbTree::last`](rb_tree.h.md#fn-20a28875ac4ed181)
- [`xv::support::RbTree::lower_bound`](rb_tree.h.md#fn-46a8a5cc0b6991b7)
- [`xv::support::RbTree::minimum`](rb_tree.h.md#fn-dc7a8f4154eafdde)
- [`xv::support::RbTree::next`](rb_tree.h.md#fn-8a42e5ed21ef03a5)
- [`xv::support::RbTree::operator=`](rb_tree.h.md#fn-675cf00ccb9eb073)
- [`xv::support::RbTree::previous`](rb_tree.h.md#fn-304e158f3fd159e6)
- [`xv::support::RbTree::red`](rb_tree.h.md#fn-958204e74541da02)
- [`xv::support::RbTree::replace`](rb_tree.h.md#fn-615d91882f7540d2)
- [`xv::support::RbTree::root`](rb_tree.h.md#fn-b2af70ae9b97b061)
- [`xv::support::RbTree::rotate_left`](rb_tree.h.md#fn-88b992824c0eb6ad)
- [`xv::support::RbTree::rotate_right`](rb_tree.h.md#fn-505cf82682ed74b9)
- [`xv::support::RbTree::update`](rb_tree.h.md#fn-824c8b71ce4de059)
- [`xv::support::RbTree::update_up`](rb_tree.h.md#fn-b244710753e506e8)

<a id="fn-bbe9b658eca338ad"></a>

## `xv::support::RbTree::RbTree<Node, Compare>`

```doxygen
@fn constexpr xv::support::RbTree<Node, Compare>::RbTree()
@brief 构造空树；复制构造被显式删除，防止复制侵入式链接。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-b2d2d3c83e01b68e"></a>

## `xv::support::RbTree::RbTree<Node, Compare>`

```doxygen
@fn xv::support::RbTree<Node, Compare>::RbTree(const RbTree &)
@brief 构造空树；复制构造被显式删除，防止复制侵入式链接。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-675cf00ccb9eb073"></a>

## `xv::support::RbTree::operator=`

```doxygen
@fn RbTree &xv::support::RbTree::operator=(const RbTree &)
@brief 禁止赋值复制树，节点所有权与链接不能隐式共享。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-b2af70ae9b97b061"></a>

## `xv::support::RbTree::root`

```doxygen
@fn Node *xv::support::RbTree::root() const
@brief 返回当前根节点；空树返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-d72d79319fd7ec4c"></a>

## `xv::support::RbTree::first`

```doxygen
@fn Node *xv::support::RbTree::first() const
@brief 沿左子树取得最小节点；空树返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-20a28875ac4ed181"></a>

## `xv::support::RbTree::last`

```doxygen
@fn Node *xv::support::RbTree::last() const
@brief 沿右子树取得最大节点；空树返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-8a42e5ed21ef03a5"></a>

## `xv::support::RbTree::next`

```doxygen
@fn static Node *xv::support::RbTree::next(Node *node)
@brief 利用右子树或父链接取得后继；末尾返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-304e158f3fd159e6"></a>

## `xv::support::RbTree::previous`

```doxygen
@fn static Node *xv::support::RbTree::previous(Node *node)
@brief 利用左子树或父链接取得前驱；首端返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-46a8a5cc0b6991b7"></a>

## `xv::support::RbTree::lower_bound`

```doxygen
@fn Node *xv::support::RbTree::lower_bound(const Node &key) const
@brief 查找首个不小于给定键的节点，最坏 O(log N)。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-9e274d7f038a163f"></a>

## `xv::support::RbTree::find`

```doxygen
@fn Node *xv::support::RbTree::find(const Node &key) const
@brief 通过 lower_bound 和比较器取得相等键，不存在时返回空指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-db5911cce7ad202a"></a>

## `xv::support::RbTree::insert`

```doxygen
@fn bool xv::support::RbTree::insert(Node &node)
@brief 插入调用方持有的稳定节点；重复键返回 false 且不修改节点，成功后修复颜色、黑高和聚合值。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-6bded9bb25d0e1e1"></a>

## `xv::support::RbTree::erase`

```doxygen
@fn void xv::support::RbTree::erase(Node &node)
@brief 删除确实属于本树的节点，处理后继替换及黑高修复，断开原节点链接；不释放节点。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-958204e74541da02"></a>

## `xv::support::RbTree::red`

```doxygen
@fn static bool xv::support::RbTree::red(Node *node)
@brief 空指针视为黑色，其他节点返回红色标志。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-dc7a8f4154eafdde"></a>

## `xv::support::RbTree::minimum`

```doxygen
@fn static Node *xv::support::RbTree::minimum(Node *node)
@brief 沿左链接取得给定子树最小节点。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-824c8b71ce4de059"></a>

## `xv::support::RbTree::update`

```doxygen
@fn static void xv::support::RbTree::update(Node *node)
@brief 节点提供 update() 时重算其子树聚合值；空节点或未提供钩子时不操作。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-b244710753e506e8"></a>

## `xv::support::RbTree::update_up`

```doxygen
@fn static void xv::support::RbTree::update_up(Node *node)
@brief 从给定节点向根更新子树聚合值。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-615d91882f7540d2"></a>

## `xv::support::RbTree::replace`

```doxygen
@fn void xv::support::RbTree::replace(Node *old, Node *node)
@brief 将一个节点在父链接或根位置替换为另一节点，并维护父指针。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-88b992824c0eb6ad"></a>

## `xv::support::RbTree::rotate_left`

```doxygen
@fn void xv::support::RbTree::rotate_left(Node *node)
@brief 左旋并更新被旋转两个节点的聚合值，保持中序顺序。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-505cf82682ed74b9"></a>

## `xv::support::RbTree::rotate_right`

```doxygen
@fn void xv::support::RbTree::rotate_right(Node *node)
@brief 右旋并更新被旋转两个节点的聚合值，保持中序顺序。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-a560d2326a30a61d"></a>

## `xv::support::RbTree::insert_fix`

```doxygen
@fn void xv::support::RbTree::insert_fix(Node *node)
@brief 迭代处理红父节点，通过叔节点重染色或旋转恢复红黑约束，最终将根染黑。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-936f3065732be42f"></a>

## `xv::support::RbTree::erase_fix`

```doxygen
@fn void xv::support::RbTree::erase_fix(Node *node, Node *parent)
@brief 携带可能为空的替代节点及父指针，迭代处理兄弟颜色和黑高亏损，恢复红黑约束。
@details 节点存储和并发排他由调用方负责，不依赖堆、标准库或异常；旋转维护子树聚合值。
```

<a id="fn-2635bc8d52023d76"></a>

## `xv::support::RbTree::swap`

```doxygen
@fn void xv::support::RbTree::swap(RbTree &other)
@brief 交换树根而不移动节点，调用方需保证两棵树均处于排他访问期间。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```
