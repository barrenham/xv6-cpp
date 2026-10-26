# `kernel/io/object.cppm`

## 函数导航

- [`xv::io::query`](object.cppm.md#fn-1ecb0d678053c236)
- [`xv::io::Capability::Capability`](object.cppm.md#fn-ecf195f263b03d0b)
- [`xv::io::Capability::~Capability`](object.cppm.md#fn-161283e8b9341214)
- [`xv::io::DeviceControl::DeviceControl`](object.cppm.md#fn-0c81a281ea490340)
- [`xv::io::DeviceControl::ioctl`](object.cppm.md#fn-b3f6d9fd9cc29333)
- [`xv::io::DeviceControl::~DeviceControl`](object.cppm.md#fn-172825626cc40af4)
- [`xv::io::IoObject::IoObject`](object.cppm.md#fn-01e3dc595dd66221)
- [`xv::io::IoObject::IoObject`](object.cppm.md#fn-eeb824198b3705fe)
- [`xv::io::IoObject::capability`](object.cppm.md#fn-4df1fbe140514161)
- [`xv::io::IoObject::close`](object.cppm.md#fn-316eabe9e9d74fce)
- [`xv::io::IoObject::operator=`](object.cppm.md#fn-e7411405250e1f87)
- [`xv::io::IoObject::poll`](object.cppm.md#fn-56aed24c02747ed9)
- [`xv::io::IoObject::read`](object.cppm.md#fn-651c3f8d7273e79e)
- [`xv::io::IoObject::stat`](object.cppm.md#fn-ddfd4e22b6380097)
- [`xv::io::IoObject::write`](object.cppm.md#fn-b26276ebef21c6da)
- [`xv::io::IoObject::~IoObject`](object.cppm.md#fn-58e61b8f03e337fd)

<a id="fn-ecf195f263b03d0b"></a>

## `xv::io::Capability::Capability`

```doxygen
@fn xv::io::Capability::Capability()
@brief 构造 Capability。
@details 只构造抽象接口的基类部分，不分配资源或执行 I/O；构造/析构通过具体派生对象进行。
```

<a id="fn-161283e8b9341214"></a>

## `xv::io::Capability::~Capability`

```doxygen
@fn xv::io::Capability::~Capability()
@brief 结束 Capability 的接口基类生命周期。
@details 受保护非虚析构禁止从基类 delete；拥有者通过明确的 close 契约管理对象。
```

<a id="fn-b3f6d9fd9cc29333"></a>

## `xv::io::DeviceControl::ioctl`

```doxygen
@fn int xv::io::DeviceControl::ioctl(unsigned request, Buffer argument)
@brief 设备控制的独立能力接口，request 与用户 Buffer 由后端解释。
@details 设备控制的独立能力接口，request 与用户 Buffer 由后端解释。没有控制语义的 inode、管道与 socket 不必实现此接口。
```

<a id="fn-0c81a281ea490340"></a>

## `xv::io::DeviceControl::DeviceControl`

```doxygen
@fn xv::io::DeviceControl::DeviceControl()
@brief 构造 DeviceControl。
@details 只构造抽象接口的基类部分，不分配资源或执行 I/O；构造/析构通过具体派生对象进行。
```

<a id="fn-172825626cc40af4"></a>

## `xv::io::DeviceControl::~DeviceControl`

```doxygen
@fn xv::io::DeviceControl::~DeviceControl()
@brief 结束 DeviceControl 的接口基类生命周期。
@details 受保护非虚析构禁止从基类 delete；拥有者通过明确的 close 契约管理对象。
```

<a id="fn-eeb824198b3705fe"></a>

## `xv::io::IoObject::IoObject`

```doxygen
@fn xv::io::IoObject::IoObject(const IoObject &)
@brief 禁止复制统一 I/O 后端。
@details 复制成员在编译期删除，防止重复关闭资源或复制打开对象的唯一状态。
```

<a id="fn-e7411405250e1f87"></a>

## `xv::io::IoObject::operator=`

```doxygen
@fn IoObject &xv::io::IoObject::operator=(const IoObject &)
@brief 禁止复制统一 I/O 后端。
@details 复制成员在编译期删除，防止重复关闭资源或复制打开对象的唯一状态。
```

<a id="fn-651c3f8d7273e79e"></a>

## `xv::io::IoObject::read`

```doxygen
@fn int xv::io::IoObject::read(Buffer destination, int count)
@brief 统一后端读取契约：用户 Buffer 含地址与非阻塞标志，count 为最大长度，返回字节数、EOF0或负错误。
@details 统一后端读取契约：用户 Buffer 含地址与非阻塞标志，count 为最大长度，返回字节数、EOF0或负错误。具体资源保存自己的条件锁、缓冲及位置。
```

<a id="fn-b26276ebef21c6da"></a>

## `xv::io::IoObject::write`

```doxygen
@fn int xv::io::IoObject::write(Buffer source, int count)
@brief 统一后端写入契约：可返回实际部分长度或负错误；具体协议决定字节流/数据报语义，不访问 File 内部字段。
@details 统一后端写入契约：可返回实际部分长度或负错误；具体协议决定字节流/数据报语义，不访问 File 内部字段。
```

<a id="fn-56aed24c02747ed9"></a>

## `xv::io::IoObject::poll`

```doxygen
@fn unsigned xv::io::IoObject::poll(xv::sync::PollTable *table)
@brief 在具体后端条件锁内先登记 PollTable 通道，再读取就绪状态；不睡眠、不消费数据。
@details 在具体后端条件锁内先登记 PollTable 通道，再读取就绪状态；不睡眠、不消费数据。File 统一进行访问权限过滤。
```

<a id="fn-ddfd4e22b6380097"></a>

## `xv::io::IoObject::stat`

```doxygen
@fn int xv::io::IoObject::stat(Metadata &)
@brief 可选元数据契约，默认返回 -1。
@details 可选元数据契约，默认返回 -1。支持的后端填充内核 Metadata 并返回0，由 File 在后端解锁后完成用户复制。
```

<a id="fn-4df1fbe140514161"></a>

## `xv::io::IoObject::capability`

```doxygen
@fn Capability *xv::io::IoObject::capability(CapabilityId)
@brief 默认没有附加能力，返回 nullptr。
@details 默认没有附加能力，返回 nullptr。具体实现只对自己真正实现的 CapabilityId 返回正确的能力基类子对象。
```

<a id="fn-316eabe9e9d74fce"></a>

## `xv::io::IoObject::close`

```doxygen
@fn void xv::io::IoObject::close()
@brief 统一最后引用关闭契约，File 恰好调用一次且不持文件表锁；后端可立即释放、显式析构内置对象，或把异步关闭交给协议注册表。
@details 统一最后引用关闭契约，File 恰好调用一次且不持文件表锁；后端可立即释放、显式析构内置对象，或把异步关闭交给协议注册表。File 槽直到此调用返回前都保留。
```

<a id="fn-01e3dc595dd66221"></a>

## `xv::io::IoObject::IoObject`

```doxygen
@fn xv::io::IoObject::IoObject()
@brief 构造 IoObject。
@details 只构造抽象接口的基类部分，不分配资源或执行 I/O；构造/析构通过具体派生对象进行。
```

<a id="fn-58e61b8f03e337fd"></a>

## `xv::io::IoObject::~IoObject`

```doxygen
@fn xv::io::IoObject::~IoObject()
@brief 结束 IoObject 的接口基类生命周期。
@details 受保护非虚析构禁止从基类 delete；拥有者通过明确的 close 契约管理对象。
```

<a id="fn-1ecb0d678053c236"></a>

## `xv::io::query`

```doxygen
@fn Interface *xv::io::query(IoObject *object)
@brief 模板化能力查询：使用 Interface::Id 向 IoObject 请求 Capability 指针，只在命中时 static_cast 回明确的接口类型；空对象/未支持能力返回 nullptr。
@details 模板化能力查询：使用 Interface::Id 向 IoObject 请求 Capability 指针，只在命中时 static_cast 回明确的接口类型；空对象/未支持能力返回 nullptr。依赖实现者遵守 ID 与接口类型对应约定，无 RTTI。
```
