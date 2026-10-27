#pragma once

#ifndef __ASSEMBLER__
#include "kernel/support/types.h"
typedef uint64 pte_t;
typedef uint64 *pagetable_t;
#endif

#define MSTATUS_MPP_MASK (3L << 11) // previous mode.
#define MSTATUS_MPP_M    (3L << 11)
#define MSTATUS_MPP_S    (1L << 11)
#define MSTATUS_MPP_U    (0L << 11)
#define SSTATUS_SPP  (1L << 8) // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE  (1L << 1) // Supervisor Interrupt Enable
#define SSTATUS_UIE  (1L << 0) // User Interrupt Enable
#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define MIE_MSIE (1L << 3) // software intr
#define MENVCFG_STCE (1L << 63)
#define MENVCFG_ADUE (1L << 61)
#define SATP_SV39 (8L << 60)
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

#define PGSIZE  4096 // bytes per page
#define PGSHIFT 12   // bits of offset within a page

#ifdef LAB_PGTBL
#define SUPERPGSIZE          (2 * (1 << 20)) // bytes per page
#define SUPERPGROUNDUP(sz)   (((sz) + SUPERPGSIZE - 1) & ~(SUPERPGSIZE - 1))
#define SUPERPGROUNDDOWN(sz) ((sz) & ~(SUPERPGSIZE - 1))
#endif

#define PGROUNDUP(sz)  (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4) // user can access
#define PTE_COW (1L << 8) // software: writable private mapping shared by fork
#define PTE_A (1L << 6) // hardware sets this when the page is accessed



#if defined(LAB_MMAP) || defined(LAB_PGTBL) || defined(LAB_COW)
#define PTE_LEAF(pte) (((pte) & PTE_R) | ((pte) & PTE_W) | ((pte) & PTE_X))
#endif

// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)

#define PTE2PA(pte) (((pte) >> 10) << 12)

#define PTE_FLAGS(pte) ((pte) & 0x3FF)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK         0x1FF // 9 bits
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va)  ((((uint64)(va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
