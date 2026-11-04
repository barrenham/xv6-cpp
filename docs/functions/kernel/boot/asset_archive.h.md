# kernel/boot/asset_archive.h

不可变引导资产及插件加载接口。

<a id="fn-70fc5a0b6498af01"></a>

```doxygen
@fn xv::bootdata::Archive::Archive(const void *data, unsigned long size)
@brief 保存借用字节和长度，不验证、不分配、不复制；调用方保证底层引导镜像仍被保留。
@details 保存借用字节和长度，不验证、不分配、不复制；调用方保证底层引导镜像仍被保留。
```

<a id="fn-ce469f3a9abe6215"></a>

```doxygen
@fn bool xv::bootdata::Archive::valid() const
@brief 验证尾部版本、目录条目大小及数量、目录边界；逐项检查可打印且终止的唯一名称、非零内容、有界偏移、允许的执行标志及保留字段。按减法比较避免加法溢出；不识别文件系统。
@details 验证尾部版本、目录条目大小及数量、目录边界；逐项检查可打印且终止的唯一名称、非零内容、有界偏移、允许的执行标志及保留字段。按减法比较避免加法溢出；不识别文件系统。
```

<a id="fn-2e188b64918036a3"></a>

```doxygen
@fn bool xv::bootdata::Archive::get(unsigned index, const char *name, Asset &out) const
@brief 先验证完整目录，再按非空名称或索引查找；成功填充借用名称/字节/长度/标志，失败不发布对象。返回字节只在引导内存保留期间有效。
@details 先验证完整目录，再按非空名称或索引查找；成功填充借用名称/字节/长度/标志，失败不发布对象。返回字节只在引导内存保留期间有效。
```

<a id="fn-f9cf33131384ee1f"></a>

```doxygen
@fn static unsigned long xv::bootdata::Archive::word(const unsigned char *p, unsigned size)
@brief 按小端顺序读取最多八字节，调用点先确保相应字段在目录或尾部范围内。
@details 按小端顺序读取最多八字节，调用点先确保相应字段在目录或尾部范围内。
```

<a id="fn-da573404c7c4d2e7"></a>

```doxygen
@fn static bool xv::bootdata::Archive::same(const char *a, const char *b)
@brief 在 NameSize 内比较 NUL 终止名称；valid 先保证存储名称有终止符。
@details 在 NameSize 内比较 NUL 终止名称；valid 先保证存储名称有终止符。
```
