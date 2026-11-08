#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/support/param.h"
#include "user/runtime/threads/internal.h"

extern "C" {

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;
static unsigned allocator_lock;

static void
free_unlocked(void *ap) {
  Header *bp, *p;

  bp = (Header *)ap - 1;
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if (bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header *
morecore(uint nu)
{
  char *p;
  Header *hp;

  if (nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if (p == SBRK_ERROR)
    return 0;
  hp = (Header *)p;
  hp->s.size = nu;
  free_unlocked((void *)(hp + 1));
  return freep;
}

static void *
malloc_unlocked(uint nbytes) {
  Header *p, *prevp;
  uint nunits;

  if (nbytes > 0x7fffffffU - sizeof(Header) * 2)
  {
    return nullptr;
  }
  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if ((prevp = freep) == 0) {
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void *)(p + 1);
    }
    if (p == freep)
      if ((p = morecore(nunits)) == 0)
        return 0;
  }
}

void
free(void *pointer) {
  if (!pointer)
  {
    return;
  }
  xv_threads::lock(allocator_lock);
  free_unlocked(pointer);
  xv_threads::unlock(allocator_lock);
}

void *
malloc(uint bytes) {
  xv_threads::lock(allocator_lock);
  auto *pointer = malloc_unlocked(bytes);
  xv_threads::unlock(allocator_lock);
  if (!pointer)
  {
    errno = ENOMEM;
  }
  return pointer;
}
}
