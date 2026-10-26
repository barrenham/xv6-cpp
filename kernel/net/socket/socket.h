#pragma once
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_NONBLOCK 0x800
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define SOL_SOCKET 1
#define SO_ERROR 4
#define MSG_DONTWAIT 0x40
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
#define INADDR_ANY 0U
#define INADDR_LOOPBACK 0x7f000001U
typedef unsigned socklen_t;
struct in_addr { unsigned s_addr; };
struct sockaddr { unsigned short sa_family; char sa_data[14]; };
struct sockaddr_in
{
  unsigned short sin_family;
  unsigned short sin_port;
  in_addr sin_addr;
  unsigned char sin_zero[8];
};
static inline unsigned short htons(unsigned short n) { return __builtin_bswap16(n); }
static inline unsigned short ntohs(unsigned short n) { return __builtin_bswap16(n); }
static inline unsigned htonl(unsigned n) { return __builtin_bswap32(n); }
static inline unsigned ntohl(unsigned n) { return __builtin_bswap32(n); }
