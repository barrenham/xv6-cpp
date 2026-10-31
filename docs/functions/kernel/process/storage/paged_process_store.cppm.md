# `kernel/process/storage/paged_process_store.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/storage/paged_process_store.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:paged_store`。

直接模块依赖：`:store`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::PagedProcessStore::PagedProcessStore`](paged_process_store.cppm.md#fn-0ca5973a6b64ff3d)
- [`xv::process::PagedProcessStore::allocate`](paged_process_store.cpp.md#fn-8add4ef4af29f162)
- [`xv::process::PagedProcessStore::count`](paged_process_store.cpp.md#fn-06351df5779372e3)
- [`xv::process::PagedProcessStore::find`](paged_process_store.cpp.md#fn-bffa9ef733537f45)
- [`xv::process::PagedProcessStore::initialize`](paged_process_store.cpp.md#fn-06625233c22e977d)
- [`xv::process::PagedProcessStore::name`](paged_process_store.cppm.md#fn-42781b830095fac8)
- [`xv::process::PagedProcessStore::node_at`](paged_process_store.cpp.md#fn-78087e7747e4c8ef)
- [`xv::process::PagedProcessStore::process_at`](paged_process_store.cpp.md#fn-9fad93a93800939d)
- [`xv::process::PagedProcessStore::release`](paged_process_store.cpp.md#fn-b890f202b0802620)
- [`xv::process::PagedProcessStore::visit`](paged_process_store.cpp.md#fn-6cb9aa1e15885253)

- [`xv::process::PagedProcessStore::Less::operator()`](paged_process_store.cppm.md#fn-9be36f15de94b44c)

<a id="fn-0ca5973a6b64ff3d"></a>

## `xv::process::PagedProcessStore::PagedProcessStore`

```doxygen
@fn constexpr xv::process::PagedProcessStore::PagedProcessStore()
@brief 构造稳定的分页进程登记，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-42781b830095fac8"></a>

## `xv::process::PagedProcessStore::name`

```doxygen
@fn const char *xv::process::PagedProcessStore::name() const override
@brief 稳定的分页进程登记：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9be36f15de94b44c"></a>

## `xv::process::PagedProcessStore::Less::operator()`

```doxygen
@fn bool xv::process::PagedProcessStore::Less::operator()(const Node &a, const Node &b) const
@brief 按节点中发布后保持不变的 indexed_pid 排序；不使用回收期间会被清零的 Process::pid 作为树键。
@note 详细锁协议见 docs/process-index-wait.md。
```
