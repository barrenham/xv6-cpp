#pragma once

namespace xv::ext3
{
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;
static_assert(sizeof(u16)==2 && sizeof(u32)==4 && sizeof(u64)==8);
inline u16 le16(const void *memory)
{ auto *p=static_cast<const u8 *>(memory);return u16(p[0])|(u16(p[1])<<8); }
inline u32 le32(const void *memory)
{ auto *p=static_cast<const u8 *>(memory);return u32(p[0])|(u32(p[1])<<8)|(u32(p[2])<<16)|(u32(p[3])<<24); }
inline u32 be32(const void *memory)
{ auto *p=static_cast<const u8 *>(memory);return (u32(p[0])<<24)|(u32(p[1])<<16)|(u32(p[2])<<8)|p[3]; }
inline void put16(void *memory,u16 value)
{ auto *p=static_cast<u8 *>(memory);p[0]=value;p[1]=value>>8; }
inline void put32(void *memory,u32 value)
{ auto *p=static_cast<u8 *>(memory);for(unsigned i=0;i<4;++i)p[i]=value>>(i*8); }
inline void put_be32(void *memory,u32 value)
{ auto *p=static_cast<u8 *>(memory);for(unsigned i=0;i<4;++i)p[3-i]=value>>(i*8); }
inline void copy(void *destination,const void *source,unsigned count)
{ auto *d=static_cast<u8 *>(destination);auto *s=static_cast<const u8 *>(source);while(count--)*d++=*s++; }
inline void zero(void *memory,unsigned count)
{ auto *p=static_cast<u8 *>(memory);while(count--)*p++=0; }
inline bool equal(const void *first,const void *second,unsigned count)
{ auto *a=static_cast<const u8 *>(first),*b=static_cast<const u8 *>(second);while(count--)if(*a++!=*b++)return false;return true; }
inline bool sequence_at_least(u32 value,u32 previous)
{ return !((value-previous)&0x80000000U); }
inline constexpr u16 Magic=0xef53;
inline constexpr u32 JournalMagic=0xc03b3998;
inline constexpr u32 HasJournal=0x4, FileType=0x2, Recover=0x4, JournalDevice=0x8;
inline constexpr u16 Directory=0x4000, Regular=0x8000, Symlink=0xa000, Character=0x2000, Block=0x6000;
inline constexpr u32 Descriptor=1, Commit=2, JournalSuperV1=3, JournalSuperV2=4, Revoke=5;
inline constexpr u32 Escaped=1, SameUuid=2, Deleted=4, LastTag=8;
enum class Error { None, Io, Corrupt, Unsupported, NoMemory, NoSpace, Invalid, NotFound, Exists, NotEmpty, ReadOnly };
}
