# kernel/fs/file/boot_asset.cpp

不可变引导资产及插件加载接口。

<a id="fn-2427ab46ab7c5b79"></a>

```doxygen
@fn explicit xv::fs::BootAsset::BootAsset(xv::bootdata::Archive::Asset asset)
@brief 保存已验证资产视图并初始化偏移锁；资产字节由启动时保留的物理页提供，无私有可写副本。
@details 保存已验证资产视图并初始化偏移锁；资产字节由启动时保留的物理页提供，无私有可写副本。
```

<a id="fn-cc594f3031cac24a"></a>

```doxygen
@fn int xv::fs::BootAsset::read(xv::io::Buffer destination, int count)
@brief 在睡眠锁内将读取限制到剩余字节；用户复制成功后才推进共享偏移，EOF 返回零，失败返回负数。dup/fork 共享偏移。
@details 在睡眠锁内将读取限制到剩余字节；用户复制成功后才推进共享偏移，EOF 返回零，失败返回负数。dup/fork 共享偏移。
```

<a id="fn-1c4364d6941f5a4b"></a>

```doxygen
@fn int xv::fs::BootAsset::write(xv::io::Buffer, int)
@brief 资产不可变，所有写请求返回负数，不修改数据或偏移。
@details 资产不可变，所有写请求返回负数，不修改数据或偏移。
```

<a id="fn-51f0697f4769d69b"></a>

```doxygen
@fn unsigned xv::fs::BootAsset::poll(xv::sync::PollTable *)
@brief 不可变字节流始终可读；EOF 也属于可立即完成的读取，不注册设备等待。
@details 不可变字节流始终可读；EOF 也属于可立即完成的读取，不注册设备等待。
```

<a id="fn-db716d66b80c33e5"></a>

```doxygen
@fn long xv::fs::BootAsset::seek(long delta, int origin)
@brief 在睡眠锁内按 SET/CUR/END 计算基础偏移，只接受 0..size，检查加法前验证上下界。
@details 在睡眠锁内按 SET/CUR/END 计算基础偏移，只接受 0..size，检查加法前验证上下界。
```

<a id="fn-cb3b0c1885b74291"></a>

```doxygen
@fn unsigned long xv::fs::BootAsset::position() const
@brief 原子读取共享偏移用于观察，不取得数据所有权。
@details 原子读取共享偏移用于观察，不取得数据所有权。
```

<a id="fn-e7c643346ed1779a"></a>

```doxygen
@fn int xv::fs::BootAsset::stat(xv::io::Metadata &out)
@brief 填充普通只读文件元数据，长度来自资产目录；可执行资产报告 0755，否则为 0644。
@details 填充普通只读文件元数据，长度来自资产目录；可执行资产报告 0755，否则为 0644。
```

<a id="fn-f1804242b4bfd8d2"></a>

```doxygen
@fn xv::io::Capability *xv::fs::BootAsset::capability(xv::io::CapabilityId id)
@brief 只有执行标志已设置且查询标识为 ExecutableData 时返回执行能力，其他查询返回空。
@details 只有执行标志已设置且查询标识为 ExecutableData 时返回执行能力，其他查询返回空。
```

<a id="fn-61ec8981c94983fe"></a>

```doxygen
@fn const void *xv::fs::BootAsset::bytes() const
@brief 返回不可变镜像字节借用；调用者必须持有 File 引用并尊重 length。
@details 返回不可变镜像字节借用；调用者必须持有 File 引用并尊重 length。
```

<a id="fn-8675418e11024b9f"></a>

```doxygen
@fn unsigned long xv::fs::BootAsset::length() const
@brief 返回已经验证的资产字节长度，不扫描或复制内容。
@details 返回已经验证的资产字节长度，不扫描或复制内容。
```

<a id="fn-26b2a04e90d22bb2"></a>

```doxygen
@fn const char *xv::fs::BootAsset::name() const
@brief 返回镜像内 NUL 终止的资产名称借用，用作执行诊断名称。
@details 返回镜像内 NUL 终止的资产名称借用，用作执行诊断名称。
```

<a id="fn-7c5f7e01e1b8323c"></a>

```doxygen
@fn void xv::fs::BootAsset::close()
@brief 销毁对象后释放对象占用的页；启动保留的资产镜像不随文件关闭释放。
@details 销毁对象后释放对象占用的页；启动保留的资产镜像不随文件关闭释放。
```

<a id="fn-88dedb5e7dee8f0e"></a>

```doxygen
@fn File *xv::fs::open_boot_asset(unsigned long address)
@brief 复制并校验版本/大小/名称的查询，按名称或索引定位已验证资产并返回元数据；创建只读对象，页面分配失败释放文件槽。调用方负责权限检查及描述符安装。
@details 复制并校验版本/大小/名称的查询，按名称或索引定位已验证资产并返回元数据；创建只读对象，页面分配失败释放文件槽。调用方负责权限检查及描述符安装。
```
