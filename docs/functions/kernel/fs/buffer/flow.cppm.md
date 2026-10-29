# `kernel/fs/buffer/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::PageBuffer::PageFlow::allocate`](flow.cppm.md#fn-333f13b2e90006c5)
- [`xv::fs::PageBuffer::PageFlow::last`](flow.cppm.md#fn-3bf78352595be26a)
- [`xv::fs::PageBuffer::PageFlow::link`](flow.cppm.md#fn-3bb349b8437cb9cd)
- [`xv::fs::PageBuffer::PageFlow::needs_page`](flow.cppm.md#fn-4a80c2e14155a81a)
- [`xv::fs::PageBuffer::PageFlow::tail`](flow.cppm.md#fn-1905c0622906d128)
- [`xv::fs::PageBuffer::PageFlow::traverse`](flow.cppm.md#fn-a6ddb574f9d5b2ee)
- [`xv::fs::PageBuffer::PageFlow::valid`](flow.cppm.md#fn-e3b356a982b66b34)
- [`xv::fs::PageBuffer::ReadFlow::clamp`](flow.cppm.md#fn-5638ea4d54dc064b)
- [`xv::fs::PageBuffer::ReadFlow::copy`](flow.cppm.md#fn-6802b89e8e5dcf0d)
- [`xv::fs::PageBuffer::ReadFlow::failure`](flow.cppm.md#fn-83aff5a5fd27243d)
- [`xv::fs::PageBuffer::ReadFlow::in_range`](flow.cppm.md#fn-8da36895c512b993)
- [`xv::fs::PageBuffer::ReadFlow::more`](flow.cppm.md#fn-83c3c8c225f6f34a)
- [`xv::fs::PageBuffer::ReadFlow::progress`](flow.cppm.md#fn-bf20fadccc42dd03)
- [`xv::fs::PageBuffer::ReadFlow::select_block`](flow.cppm.md#fn-77b94cc71ab25fce)
- [`xv::fs::PageBuffer::ResizeFlow::commit`](flow.cppm.md#fn-fb568172fd52cd93)
- [`xv::fs::PageBuffer::ResizeFlow::extend`](flow.cppm.md#fn-2694c139b464ce46)
- [`xv::fs::PageBuffer::ResizeFlow::growing`](flow.cppm.md#fn-f2a5b1462c0c8dd6)
- [`xv::fs::PageBuffer::ResizeFlow::release_tail`](flow.cppm.md#fn-b4014cf174a43b5c)
- [`xv::fs::PageBuffer::ResizeFlow::select_tail`](flow.cppm.md#fn-e170fc112059c6e2)
- [`xv::fs::PageBuffer::ResizeFlow::valid`](flow.cppm.md#fn-98070f1db3f02c4d)
- [`xv::fs::PageBuffer::WriteFlow::complete`](flow.cppm.md#fn-3e051771f85ecf68)
- [`xv::fs::PageBuffer::WriteFlow::copy`](flow.cppm.md#fn-7a9ee115683179bb)
- [`xv::fs::PageBuffer::WriteFlow::fill_gap`](flow.cppm.md#fn-f4f0cc808a2b6a74)
- [`xv::fs::PageBuffer::WriteFlow::gap`](flow.cppm.md#fn-2bd1673b90b99067)
- [`xv::fs::PageBuffer::WriteFlow::more`](flow.cppm.md#fn-c68347e57501b359)
- [`xv::fs::PageBuffer::WriteFlow::progress`](flow.cppm.md#fn-739571a25f1f311a)
- [`xv::fs::PageBuffer::WriteFlow::select_block`](flow.cppm.md#fn-6d2e20ed20daac11)
- [`xv::fs::PageBuffer::WriteFlow::valid`](flow.cppm.md#fn-e1510177b6348fc4)

<a id="fn-333f13b2e90006c5"></a>

## `xv::fs::PageBuffer::PageFlow::allocate`

```doxygen
@fn static bool xv::fs::PageBuffer::PageFlow::allocate(Context &c) noexcept
@brief 分配一页并原位初始化 Page。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3bf78352595be26a"></a>

## `xv::fs::PageBuffer::PageFlow::last`

```doxygen
@fn static bool xv::fs::PageBuffer::PageFlow::last(Context &c) noexcept
@brief 检查目标是否为尾页。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3bb349b8437cb9cd"></a>

## `xv::fs::PageBuffer::PageFlow::link`

```doxygen
@fn static void xv::fs::PageBuffer::PageFlow::link(Context &c) noexcept
@brief 将新页挂到尾部并增加拥有页数。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-4a80c2e14155a81a"></a>

## `xv::fs::PageBuffer::PageFlow::needs_page`

```doxygen
@fn static bool xv::fs::PageBuffer::PageFlow::needs_page(Context &c) noexcept
@brief 检查 create 策略下是否还需要扩充到目标页。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-1905c0622906d128"></a>

## `xv::fs::PageBuffer::PageFlow::tail`

```doxygen
@fn static void xv::fs::PageBuffer::PageFlow::tail(Context &c) noexcept
@brief 保存尾页作为查找结果。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-a6ddb574f9d5b2ee"></a>

## `xv::fs::PageBuffer::PageFlow::traverse`

```doxygen
@fn static void xv::fs::PageBuffer::PageFlow::traverse(Context &c) noexcept
@brief 直接遍历链表到指定页并保存结果。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e3b356a982b66b34"></a>

## `xv::fs::PageBuffer::PageFlow::valid`

```doxygen
@fn static bool xv::fs::PageBuffer::PageFlow::valid(Context &c) noexcept
@brief 检查目标索引是否已在拥有页范围内。
@details 页面由 PageBuffer 拥有；分配失败保留此前已挂接的页面，链表查找保持直接循环。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-5638ea4d54dc064b"></a>

## `xv::fs::PageBuffer::ReadFlow::clamp`

```doxygen
@fn static void xv::fs::PageBuffer::ReadFlow::clamp(Context &c) noexcept
@brief 将读取计数裁剪到缓冲末尾。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-6802b89e8e5dcf0d"></a>

## `xv::fs::PageBuffer::ReadFlow::copy`

```doxygen
@fn static bool xv::fs::PageBuffer::ReadFlow::copy(Context &c) noexcept
@brief 将本段数据复制到指定地址空间。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-83aff5a5fd27243d"></a>

## `xv::fs::PageBuffer::ReadFlow::failure`

```doxygen
@fn static void xv::fs::PageBuffer::ReadFlow::failure(Context &c) noexcept
@brief 记录首段失败或此前完整完成的字节数。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-8da36895c512b993"></a>

## `xv::fs::PageBuffer::ReadFlow::in_range`

```doxygen
@fn static bool xv::fs::PageBuffer::ReadFlow::in_range(Context &c) noexcept
@brief 检查读取起点是否位于逻辑长度内。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-83c3c8c225f6f34a"></a>

## `xv::fs::PageBuffer::ReadFlow::more`

```doxygen
@fn static bool xv::fs::PageBuffer::ReadFlow::more(Context &c) noexcept
@brief 检查是否还有可读取数据。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-bf20fadccc42dd03"></a>

## `xv::fs::PageBuffer::ReadFlow::progress`

```doxygen
@fn static void xv::fs::PageBuffer::ReadFlow::progress(Context &c) noexcept
@brief 累计完整复制的字节数。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-77b94cc71ab25fce"></a>

## `xv::fs::PageBuffer::ReadFlow::select_block`

```doxygen
@fn static void xv::fs::PageBuffer::ReadFlow::select_block(Context &c) noexcept
@brief 查找数据页并计算本次页内范围，缺页诊断为内部错误。
@details 不分配页面；调用者串行化缓冲修改。页内复制失败按此前完整进展返回，首段失败返回负值。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-fb568172fd52cd93"></a>

## `xv::fs::PageBuffer::ResizeFlow::commit`

```doxygen
@fn static void xv::fs::PageBuffer::ResizeFlow::commit(Context &c) noexcept
@brief 提交保留链表与长度，并清零保留页未使用的后缀。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-2694c139b464ce46"></a>

## `xv::fs::PageBuffer::ResizeFlow::extend`

```doxygen
@fn static void xv::fs::PageBuffer::ResizeFlow::extend(Context &c) noexcept
@brief 通过写入末尾零字节尝试扩张并保存业务结果。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-f2a5b1462c0c8dd6"></a>

## `xv::fs::PageBuffer::ResizeFlow::growing`

```doxygen
@fn static bool xv::fs::PageBuffer::ResizeFlow::growing(Context &c) noexcept
@brief 检查目标是否大于当前逻辑长度。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-b4014cf174a43b5c"></a>

## `xv::fs::PageBuffer::ResizeFlow::release_tail`

```doxygen
@fn static void xv::fs::PageBuffer::ResizeFlow::release_tail(Context &c) noexcept
@brief 直接释放不再保留的尾部页面。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e170fc112059c6e2"></a>

## `xv::fs::PageBuffer::ResizeFlow::select_tail`

```doxygen
@fn static void xv::fs::PageBuffer::ResizeFlow::select_tail(Context &c) noexcept
@brief 计算保留页数并直接遍历到待释放尾链。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-98070f1db3f02c4d"></a>

## `xv::fs::PageBuffer::ResizeFlow::valid`

```doxygen
@fn static bool xv::fs::PageBuffer::ResizeFlow::valid(Context &c) noexcept
@brief 检查目标长度不超过有符号长度上限。
@details 扩张委托写入最后一个零字节完成补零；收缩释放尾部页面并清零保留页的无效后缀。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3e051771f85ecf68"></a>

## `xv::fs::PageBuffer::WriteFlow::complete`

```doxygen
@fn static void xv::fs::PageBuffer::WriteFlow::complete(Context &c) noexcept
@brief 记录本次成功完成的字节数，包括零字节写入。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-7a9ee115683179bb"></a>

## `xv::fs::PageBuffer::WriteFlow::copy`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::copy(Context &c) noexcept
@brief 从指定地址空间复制本段数据并报告是否成功。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-f4f0cc808a2b6a74"></a>

## `xv::fs::PageBuffer::WriteFlow::fill_gap`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::fill_gap(Context &c) noexcept
@brief 按页补零原末尾到写入偏移之间的间隙。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-2bd1673b90b99067"></a>

## `xv::fs::PageBuffer::WriteFlow::gap`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::gap(Context &c) noexcept
@brief 检查非空写入是否需要补零间隙。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-c68347e57501b359"></a>

## `xv::fs::PageBuffer::WriteFlow::more`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::more(Context &c) noexcept
@brief 检查是否还有数据需要复制。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-739571a25f1f311a"></a>

## `xv::fs::PageBuffer::WriteFlow::progress`

```doxygen
@fn static void xv::fs::PageBuffer::WriteFlow::progress(Context &c) noexcept
@brief 累计本段已完成字节并发布增大的缓冲长度。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-6d2e20ed20daac11"></a>

## `xv::fs::PageBuffer::WriteFlow::select_block`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::select_block(Context &c) noexcept
@brief 取得或创建目标页并计算本次页内复制范围。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e1510177b6348fc4"></a>

## `xv::fs::PageBuffer::WriteFlow::valid`

```doxygen
@fn static bool xv::fs::PageBuffer::WriteFlow::valid(Context &c) noexcept
@brief 检查计数与偏移相加不会超过有符号长度上限。
@details 每次完整复制一段后才发布新大小与已完成字节数；填隙失败返回错误，已有页面仍归缓冲拥有。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```
