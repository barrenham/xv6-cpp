# `kernel/process/exec/exec_snapshot.h`

<a id="fn-ec4050aaaf979421"></a>

## `xv::exec::Snapshot::Snapshot`

```doxygen
@fn constexpr xv::exec::Snapshot::Snapshot()=default
@brief 构造对象并建立成员初值及借用关系。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-9812b15b7cfc1941"></a>

## `xv::exec::Snapshot::~Snapshot`

```doxygen
@fn xv::exec::Snapshot::~Snapshot()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-612ed250f894cdb0"></a>

## `xv::exec::Snapshot::Snapshot`

```doxygen
@fn xv::exec::Snapshot::Snapshot(const Snapshot&)=delete
@brief 构造对象并建立成员初值及借用关系。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-3656a974c726b6b8"></a>

## `xv::exec::Snapshot::operator=`

```doxygen
@fn Snapshot &xv::exec::Snapshot::operator=(const Snapshot&)=delete
@brief 规定对象赋值/复制约束，防止资源所有权被隐式复制。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-0ad9f0c7ca1e290b"></a>

## `xv::exec::Snapshot::swap`

```doxygen
@fn void xv::exec::Snapshot::swap(Snapshot &other)
@brief 交换两个快照的页链所有权。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-1ffaf96b2e3b5a9c"></a>

## `xv::exec::Snapshot::inherit`

```doxygen
@fn void xv::exec::Snapshot::inherit(const Snapshot &source)
@brief 共享不可变页链并增加原子引用，fork不复制字符串。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-c8596197ca0c30e1"></a>

## `xv::exec::Snapshot::reset`

```doxygen
@fn void xv::exec::Snapshot::reset()
@brief 释放持有的快照/暂存资源，最后引用才归还页面。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-92c5bbf0b8875eff"></a>

## `xv::exec::Snapshot::prepare`

```doxygen
@fn bool xv::exec::Snapshot::prepare(const char *path,char *const *arguments,char *const *environment, Allocate allocate,Free release,unsigned max_arguments=32,unsigned max_environment=32)
@brief 先构造不可变路径、argv和环境候选，受数量与总字节上限约束；成功才替换旧快照。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-395c2fe534f4bfb0"></a>

## `xv::exec::Snapshot::copy`

```doxygen
@fn bool xv::exec::Snapshot::copy(Field field,void *context,Sink sink) const
@brief 将指定快照字段逐页写入sink，失败立即停止。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-498ac6413d490d30"></a>

## `xv::exec::Snapshot::Page::Page`

```doxygen
@fn explicit xv::exec::Snapshot::Page::Page(Free free):next(nullptr),release(free),references(1),path(0),arguments(0),environment(0),used(0)
@brief 构造对象并建立成员初值及借用关系。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```

<a id="fn-2bacd3bbcd62f879"></a>

## `xv::exec::Snapshot::string`

```doxygen
@fn bool xv::exec::Snapshot::string(const char *text,Allocate allocate,Free release,Page *&tail,unsigned &size,unsigned &total)
@brief 把一个含NUL字符串追加到候选页链，受总字节额度约束。
@details 不可变exec快照容器。prepare先构建候选，失败保留旧状态；fork只增加原子引用，最后引用释放整条页链。argv/env保留NUL分隔，Executable不包含终止NUL。
```
