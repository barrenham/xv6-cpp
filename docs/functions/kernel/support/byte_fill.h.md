# `kernel/support/byte_fill.h`

<a id="fn-cfdae64c12d3d113"></a>

## `xv::support::byte_fill`

```doxygen
@fn inline void *xv::support::byte_fill(void *destination, int value, unsigned long count)
@brief 按字节值填充给定区间，使用对齐机器字主体和精确前后缀。
@details 字节语义填充：前缀先对齐，主体使用may_alias机器字，尾部逐字节处理；只写入请求半开区间，长度零允许空指针。
```
