# `kernel/net/socket/socket.h`

## 函数导航

- [`htonl`](socket.h.md#fn-25a27e4090d07897)
- [`htons`](socket.h.md#fn-cfaad5c6b15bde27)
- [`ntohl`](socket.h.md#fn-1b03ec8a1ba752db)
- [`ntohs`](socket.h.md#fn-2790eaf44b07efb4)

<a id="fn-cfaad5c6b15bde27"></a>

## `htons`

```doxygen
@fn unsigned short htons(unsigned short n)
@brief 16 位主机序/网络序转换。
@param n 待转换的无符号数值。
@return 在当前小端目标上交换字节顺序后的数值。
```

<a id="fn-2790eaf44b07efb4"></a>

## `ntohs`

```doxygen
@fn unsigned short ntohs(unsigned short n)
@brief 16 位主机序/网络序转换。
@param n 待转换的无符号数值。
@return 在当前小端目标上交换字节顺序后的数值。
```

<a id="fn-25a27e4090d07897"></a>

## `htonl`

```doxygen
@fn unsigned htonl(unsigned n)
@brief 32 位主机序/网络序转换。
@param n 待转换的无符号数值。
@return 在当前小端目标上交换字节顺序后的数值。
```

<a id="fn-1b03ec8a1ba752db"></a>

## `ntohl`

```doxygen
@fn unsigned ntohl(unsigned n)
@brief 32 位主机序/网络序转换。
@param n 待转换的无符号数值。
@return 在当前小端目标上交换字节顺序后的数值。
```
