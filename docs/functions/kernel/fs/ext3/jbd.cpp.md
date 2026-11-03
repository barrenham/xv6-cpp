# `kernel/fs/ext3/jbd.cpp`

> 生产执行位置：用户态格式库。此源码也供离线工具和主机参考夹具复用，`kernel.elf` 不链接此实现。见[文件系统服务迁移](../../../../fs-service-migration.md)。

<a id="fn-2878ea6289c61dfd"></a>

## `xv::ext3::Scratch::Scratch`

```doxygen
@fn explicit xv::ext3::Scratch::Scratch(JournalIo &io):io_(io),data_(static_cast<u8 *>(io.allocate_page()))
@brief 构造对象并建立成员初值及借用关系。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-8b91a7d6d3ae144e"></a>

## `xv::ext3::Scratch::~Scratch`

```doxygen
@fn xv::ext3::Scratch::~Scratch()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-faaededb28c48d71"></a>

## `xv::ext3::Scratch::get`

```doxygen
@fn u8 *xv::ext3::Scratch::get() const
@brief 按inode号复用或取得受引用锁保护的缓存槽。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-a4479c2fb7b020ae"></a>

## `xv::ext3::header`

```doxygen
@fn void xv::ext3::header(u8 *data,unsigned bytes,u32 type,u32 sequence)
@brief 实现 xv::ext3::header 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-2307cbc976ae70fb"></a>

## `xv::ext3::Jbd::Revocations::Revocations`

```doxygen
@fn explicit xv::ext3::Jbd::Revocations::Revocations(JournalIo &device):io(device)
@brief 构造对象并建立成员初值及借用关系。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-bfd188569715b12c"></a>

## `xv::ext3::Jbd::Revocations::~Revocations`

```doxygen
@fn xv::ext3::Jbd::Revocations::~Revocations()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-f46122475d7dfa87"></a>

## `xv::ext3::Jbd::Revocations::add`

```doxygen
@fn bool xv::ext3::Jbd::Revocations::add(u32 block,u32 sequence)
@brief 校验provider名称、只读权限和容量后登记，拒绝重复及保留名称。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-4eba054c43af0c92"></a>

## `xv::ext3::Jbd::Revocations::contains`

```doxygen
@fn bool xv::ext3::Jbd::Revocations::contains(u32 block,u32 sequence) const
@brief 实现 xv::ext3::Jbd::Revocations::contains 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-fd2d7403e25d8f61"></a>

## `xv::ext3::Jbd::capacity`

```doxygen
@fn unsigned Jbd::xv::ext3::Jbd::capacity() const
@brief 查询可安全暂存的日志块容量。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-84dc32d01171adf8"></a>

## `xv::ext3::Jbd::mount`

```doxygen
@fn bool xv::ext3::Jbd::mount()
@brief 校验日志并预留提交工作页，部分预留失败释放已取得页面。
@details 生产执行位置为用户态格式库。挂载验证完成后预留三个提交工作页，部分分配失败立即回收；运行时 commit 与 write_super 复用这些页，不再在接受修改后分配临时缓冲。描述符/载荷/commit/home/super 的写入与 flush 顺序不变，真实 I/O 错误继续失败。reset 同时释放 dirty 与预留页，原恢复、撤销与序号回绕处理保留。
```

<a id="fn-f45f99ea428df3fd"></a>

## `xv::ext3::Jbd::write_super`

```doxygen
@fn bool xv::ext3::Jbd::write_super(u32 start,u32 sequence)
@brief 复用挂载时预留的日志头工作页，刷新日志起点和序号。
@details 生产执行位置为用户态格式库。挂载验证完成后预留三个提交工作页，部分分配失败立即回收；运行时 commit 与 write_super 复用这些页，不再在接受修改后分配临时缓冲。描述符/载荷/commit/home/super 的写入与 flush 顺序不变，真实 I/O 错误继续失败。reset 同时释放 dirty 与预留页，原恢复、撤销与序号回绕处理保留。
```

<a id="fn-5f83ab0e886b23b5"></a>

## `xv::ext3::Jbd::incomplete`

```doxygen
@fn bool Jbd::xv::ext3::Jbd::incomplete(unsigned pass,u32 sequence,u32 cursor)
@brief 判断日志尾部是否可作为未提交事务忽略。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-fe8f6c002e489b66"></a>

## `xv::ext3::Jbd::scan`

```doxygen
@fn bool Jbd::xv::ext3::Jbd::scan(unsigned pass,u32 &end_sequence,Revocations *revocations)
@brief 扫描标准JBD记录，核对序号、descriptor、commit和revoke。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-4aeb4495dce71ad3"></a>

## `xv::ext3::Jbd::recover`

```doxygen
@fn bool Jbd::xv::ext3::Jbd::recover()
@brief 先确认提交和revoke集合，再重放有效事务，最后清理日志头。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-c20ee5d2fed40650"></a>

## `xv::ext3::Jbd::edit`

```doxygen
@fn u8 *Jbd::xv::ext3::Jbd::edit(u32 block)
@brief 取得可修改暂存块，首次或savepoint后写入按需分配并检查credit。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-666f7c7adf93389d"></a>

## `xv::ext3::Jbd::staged`

```doxygen
@fn u8 *Jbd::xv::ext3::Jbd::staged(u32 block)
@brief 查询已暂存的数据块。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-eeea4b93975a608f"></a>

## `xv::ext3::Jbd::read`

```doxygen
@fn bool Jbd::xv::ext3::Jbd::read(u32 block,void *destination)
@brief 实现 xv::ext3::Jbd::read 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-11d9460ce91aba47"></a>

## `xv::ext3::Jbd::savepoint`

```doxygen
@fn bool Jbd::xv::ext3::Jbd::savepoint()
@brief 记录修改前暂存视图，后续edit按需复制原块。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-3790d97f4098aeb2"></a>

## `xv::ext3::Jbd::accept`

```doxygen
@fn void Jbd::xv::ext3::Jbd::accept()
@brief 接受当前savepoint并释放旧暂存版本。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-ec439701f22d506d"></a>

## `xv::ext3::Jbd::rollback`

```doxygen
@fn void Jbd::xv::ext3::Jbd::rollback()
@brief 恢复savepoint前块内容、dirty计数及错误边界。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-f9c31fc6def17b65"></a>

## `xv::ext3::Jbd::reset`

```doxygen
@fn void xv::ext3::Jbd::reset()
@brief 撤销未接受修改并释放 dirty 与挂载期预留的所有工作页。
@details 生产执行位置为用户态格式库。挂载验证完成后预留三个提交工作页，部分分配失败立即回收；运行时 commit 与 write_super 复用这些页，不再在接受修改后分配临时缓冲。描述符/载荷/commit/home/super 的写入与 flush 顺序不变，真实 I/O 错误继续失败。reset 同时释放 dirty 与预留页，原恢复、撤销与序号回绕处理保留。
```

<a id="fn-fdb68219a2e1ea1f"></a>

## `xv::ext3::Jbd::commit`

```doxygen
@fn bool xv::ext3::Jbd::commit()
@brief 用已预留缓冲执行原持久日志提交顺序，不再临时分配工作页。
@details 生产执行位置为用户态格式库。挂载验证完成后预留三个提交工作页，部分分配失败立即回收；运行时 commit 与 write_super 复用这些页，不再在接受修改后分配临时缓冲。描述符/载荷/commit/home/super 的写入与 flush 顺序不变，真实 I/O 错误继续失败。reset 同时释放 dirty 与预留页，原恢复、撤销与序号回绕处理保留。
```


- [`xv::ext3::Jbd::accept`](jbd.cpp.md#fn-3790d97f4098aeb2)

- [`xv::ext3::Jbd::capacity`](jbd.cpp.md#fn-fd2d7403e25d8f61)

- [`xv::ext3::Jbd::commit`](jbd.cpp.md#fn-fdb68219a2e1ea1f)

- [`xv::ext3::Jbd::edit`](jbd.cpp.md#fn-c20ee5d2fed40650)

- [`xv::ext3::Jbd::incomplete`](jbd.cpp.md#fn-5f83ab0e886b23b5)

- [`xv::ext3::Jbd::mount`](jbd.cpp.md#fn-84dc32d01171adf8)

- [`xv::ext3::Jbd::read`](jbd.cpp.md#fn-eeea4b93975a608f)

- [`xv::ext3::Jbd::recover`](jbd.cpp.md#fn-4aeb4495dce71ad3)

- [`xv::ext3::Jbd::reset`](jbd.cpp.md#fn-f9c31fc6def17b65)

- [`xv::ext3::Jbd::rollback`](jbd.cpp.md#fn-ec439701f22d506d)

- [`xv::ext3::Jbd::savepoint`](jbd.cpp.md#fn-11d9460ce91aba47)

- [`xv::ext3::Jbd::scan`](jbd.cpp.md#fn-fe8f6c002e489b66)

- [`xv::ext3::Jbd::staged`](jbd.cpp.md#fn-666f7c7adf93389d)

- [`xv::ext3::Jbd::write_super`](jbd.cpp.md#fn-f45f99ea428df3fd)

- [`xv::ext3::Jbd::Revocations::Revocations`](jbd.cpp.md#fn-2307cbc976ae70fb)

- [`xv::ext3::Jbd::Revocations::add`](jbd.cpp.md#fn-f46122475d7dfa87)

- [`xv::ext3::Jbd::Revocations::contains`](jbd.cpp.md#fn-4eba054c43af0c92)

- [`xv::ext3::Jbd::Revocations::~Revocations`](jbd.cpp.md#fn-bfd188569715b12c)

- [`xv::ext3::header`](jbd.cpp.md#fn-a4479c2fb7b020ae)

- [`xv::ext3::Scratch::Scratch`](jbd.cpp.md#fn-2878ea6289c61dfd)

- [`xv::ext3::Scratch::get`](jbd.cpp.md#fn-faaededb28c48d71)

- [`xv::ext3::Scratch::~Scratch`](jbd.cpp.md#fn-8b91a7d6d3ae144e)
