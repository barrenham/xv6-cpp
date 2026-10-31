#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct xv_ext3_metadata {
  uint32_t kind,uid,gid,mode,links,major,minor;
  uint64_t size;
};
unsigned xv_ext3_abi(void);
void *xv_ext3_open(void *bytes,uint64_t length,unsigned *error);
void xv_ext3_close(void *image);
unsigned xv_ext3_error(void *image);
int xv_ext3_info(void *image,uint32_t inode,struct xv_ext3_metadata *metadata);
int xv_ext3_read(void *image,uint32_t inode,uint64_t offset,void *bytes,unsigned count);
int xv_ext3_next(void *image,uint32_t inode,uint64_t *offset,char *name,unsigned *length,uint32_t *child);
int xv_ext3_attributes(void *image,uint32_t inode,uint32_t uid,uint32_t gid,unsigned mode);
int xv_ext3_create(void *image,uint32_t parent,const char *name,unsigned kind,uint32_t uid,uint32_t gid,unsigned mode,uint32_t *child);
int xv_ext3_link(void *image,uint32_t parent,const char *name,uint32_t child);
int xv_ext3_unlink(void *image,uint32_t parent,const char *name);
int xv_ext3_write(void *image,uint32_t inode,uint64_t offset,const void *bytes,uint64_t count);
int xv_ext3_device(void *image,uint32_t inode,uint32_t major,uint32_t minor);
#ifdef __cplusplus
}
#endif
