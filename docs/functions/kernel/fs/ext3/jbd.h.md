# `kernel/fs/ext3/jbd.h`

<a id="fn-540924a3a732f4a1"></a>

## `xv::ext3::JournalIo::block_size`

```doxygen
@fn virtual unsigned xv::ext3::JournalIo::block_size() const = 0
@brief 实现 xv::ext3::JournalIo::block_size 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-e0d0aa1d7ffbc817"></a>

## `xv::ext3::JournalIo::journal_capacity`

```doxygen
@fn virtual u32 xv::ext3::JournalIo::journal_capacity() const = 0
@brief 实现 xv::ext3::JournalIo::journal_capacity 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-dee8f45b6c14f927"></a>

## `xv::ext3::JournalIo::read_journal`

```doxygen
@fn virtual bool xv::ext3::JournalIo::read_journal(u32 logical,void *data) = 0
@brief 实现 xv::ext3::JournalIo::read_journal 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-7ecc70adf4f87386"></a>

## `xv::ext3::JournalIo::write_journal`

```doxygen
@fn virtual bool xv::ext3::JournalIo::write_journal(u32 logical,const void *data) = 0
@brief 实现 xv::ext3::JournalIo::write_journal 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-922a5492453f6fcb"></a>

## `xv::ext3::JournalIo::read_home`

```doxygen
@fn virtual bool xv::ext3::JournalIo::read_home(u32 block,void *data) = 0
@brief 实现 xv::ext3::JournalIo::read_home 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-340b3864ccdcd7ff"></a>

## `xv::ext3::JournalIo::write_home`

```doxygen
@fn virtual bool xv::ext3::JournalIo::write_home(u32 block,const void *data) = 0
@brief 实现 xv::ext3::JournalIo::write_home 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-4aec5a0a5b8b4a18"></a>

## `xv::ext3::JournalIo::valid_target`

```doxygen
@fn virtual bool xv::ext3::JournalIo::valid_target(u32 block) const = 0
@brief 实现 xv::ext3::JournalIo::valid_target 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-c18b73ed82b453b2"></a>

## `xv::ext3::JournalIo::flush`

```doxygen
@fn virtual bool xv::ext3::JournalIo::flush() = 0
@brief 实现 xv::ext3::JournalIo::flush 的模块操作，按声明参数读写其封装状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-211338170a90b681"></a>

## `xv::ext3::JournalIo::recovery_flag`

```doxygen
@fn virtual bool xv::ext3::JournalIo::recovery_flag(bool enabled) = 0
@brief 读写文件系统需要恢复标志并保证所需持久化顺序。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-141f0e8d822fa800"></a>

## `xv::ext3::JournalIo::allocate_page`

```doxygen
@fn virtual void *xv::ext3::JournalIo::allocate_page() = 0
@brief 优先申请普通页，仅提交阶段OOM时借用保留页。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-67b9c8c14fc64bdd"></a>

## `xv::ext3::JournalIo::free_page`

```doxygen
@fn virtual void xv::ext3::JournalIo::free_page(void *page) = 0
@brief 归还普通页或撤销提交储备借用，拒绝重复归还。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-26a471c1b0753fc9"></a>

## `xv::ext3::JournalIo::~JournalIo`

```doxygen
@fn xv::ext3::JournalIo::~JournalIo() = default
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-94dc2b0b24a3c95c"></a>

## `xv::ext3::Jbd::Jbd`

```doxygen
@fn constexpr explicit xv::ext3::Jbd::Jbd(JournalIo &io):io_(io)
@brief 构造对象并建立成员初值及借用关系。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-576f0c4549e8ec28"></a>

## `xv::ext3::Jbd::Jbd`

```doxygen
@fn xv::ext3::Jbd::Jbd(const Jbd &)=delete
@brief 构造对象并建立成员初值及借用关系。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-2010138f34011d2b"></a>

## `xv::ext3::Jbd::operator=`

```doxygen
@fn Jbd &xv::ext3::Jbd::operator=(const Jbd &)=delete
@brief 规定对象赋值/复制约束，防止资源所有权被隐式复制。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-7b5e3d6c2b984fde"></a>

## `xv::ext3::Jbd::error`

```doxygen
@fn Error xv::ext3::Jbd::error() const
@brief 查询最近一次操作错误。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-dd293ea9777a7693"></a>

## `xv::ext3::Jbd::clear_error`

```doxygen
@fn void xv::ext3::Jbd::clear_error()
@brief 清除上一操作的错误状态。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-0b532977fb24fec2"></a>

## `xv::ext3::Jbd::dirty_count`

```doxygen
@fn unsigned xv::ext3::Jbd::dirty_count() const
@brief 查询当前暂存dirty块数量。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-f03d9631e3315401"></a>

## `xv::ext3::Jbd::sequence`

```doxygen
@fn u32 xv::ext3::Jbd::sequence() const
@brief 读取当前事务序号。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-001665192e99fe3a"></a>

## `xv::ext3::Jbd::next`

```doxygen
@fn u32 xv::ext3::Jbd::next(u32 block) const
@brief 计算循环日志中的下一块位置。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```

<a id="fn-e31bb964bc923c3d"></a>

## `xv::ext3::Jbd::fail`

```doxygen
@fn bool xv::ext3::Jbd::fail(Error error)
@brief 记录错误并返回失败。
@details 标准JBD磁盘事务算法。savepoint暂存采用COW，失败回滚；提交记录持久化后才写home blocks，恢复只重放已提交且未撤销的数据。由模块适配器提供串行化。
```
