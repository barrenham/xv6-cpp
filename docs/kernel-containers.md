# 内核基础数据结构

这些 C++20 头文件不依赖标准库、异常、RTTI 或动态分配。容器不提供锁，
也不接管侵入式节点的存储；调用方负责并发排他和节点生命周期。

| 工具 | 接口与用途 | 复杂度 | 当前使用者 |
| --- | --- | --- | --- |
| `support/rb_tree.h` | 侵入式红黑树、前驱/后继、lower_bound、可选子树聚合 | 查找/修改 O(log N) | 虚拟地址池、公平调度 |
| `support/hash_table.h` | 侵入式分离链接哈希索引、整数混合与 FNV-1a 字节哈希 | 预期 O(1 + N/B)，碰撞最坏 O(N) | `ProcessGroups` 的组 ID 索引 |
| `support/intrusive_list.h` | 双向侵入式 FIFO，准确节点删除 | 插入/删除/弹出 O(1) | 调度就绪链与各 hart 的运行任务集合 |
| `support/ring_buffer.h` | 固定容量 FIFO，满时拒绝写入 | 入队/出队 O(1) | 可供设备和有界消息缓冲复用 |
| `support/bitmap.h` | 调用方存储上的位图视图、从给定位置找空位 | 单位操作 O(1)，空位搜索 O(words) | 可供固定资源集合复用 |

## 哈希索引

```cpp
#include "kernel/support/hash_table.h"

struct Entry : xv::support::HashLinks<Entry> { unsigned id; };
struct Key {
  static unsigned key(const Entry &entry) { return entry.id; }
  static unsigned long hash(unsigned id) { return xv::support::hash_integer(id); }
  static bool equal(unsigned a, unsigned b) { return a == b; }
};
xv::support::IntrusiveHashTable<Entry, unsigned, Key, 128> index;
```

桶数量 B 必须是非零二次幂，影响碰撞长度，不限制条目数量 N。
预期常数时间要求负载因子有界；此实现不自动扩桶，N/B 增大时碰撞链会变长。节点入索引期间不能移动或改变键；
重复节点或重复键的插入返回 false，不改变既有条目。`erase(node)` 按节点身份移除，
未知节点返回 false；`find(key)` 返回借用指针。释放节点前必须移除索引。
散列函数是非密码学散列；哈希表的 key/hash/equal 均由 Traits 提供。

进程组仍通过原来的全局组锁串行访问。组创建时在链表发布前登记哈希项，
最后一个成员离开时先从索引与组链表移除，再释放组页。组链表继续支持完整遍历；
成员 PID 查询仍采用原逻辑，这次没有宣称替换整个进程注册表。

## 链表、环形缓冲与位图

链表节点继承 `ListLinks<Node>`，同一套链接只能属于一张链表。`push_back` 拒绝重复入链；
`erase` 要求节点确实属于该链表，`pop_front` 在空表返回空指针。移除后可再次使用节点。

`RingBuffer<T, Capacity>` 使用内部定长数组，容量不要求是二次幂。
`push` 不覆盖未读数据，`pop` 在空时失败，`front` 返回借用指针；`clear` 丢弃逻辑内容。
它不是无锁 SPSC 队列，生产者和消费者也必须由调用方串行。

`BitmapView` 使用外部 64 位字数组，不隐式清零。调用方提供足够的有效存储，
`set/clear/test` 不越过逻辑长度；`first_clear(start)` 用逻辑位数表示未找到，
不会返回末字中逻辑范围外的填充位，也不会对零调用 `ctzl`。

## 验证

`support_containers_contract` 覆盖 2048 个条目全部落在一个桶时的查找、重复键与移除，
链表生命周期，十万次环形缓冲操作与独立 FIFO 模型比对，以及 0/1/63/64/65/127/130 位边界。
相同契约通过 ASan/UBSan；红黑树的排序、颜色、黑高和聚合验证由 `rb_tree_contract` 单独维护。
