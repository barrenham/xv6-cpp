#pragma once
extern "C" {
#pragma once
// clang-format off
#ifdef LAB_MMAP
typedef unsigned long size_t;
typedef long int off_t;
#endif
struct stat;
#ifdef LAB_LOCK
struct rwspinlock;
#endif

// kalloc.c
void*           kalloc(void);
void            kfree(void *);
#ifdef LAB_PGTBL
void*           superalloc(void);
void            superfree(void *);
void            supersplit(void *);
int             uvmdemote(pagetable_t, uint64, uint64);
#endif
void            kinit(void);

// printk.c
int             printk(char*, ...) __attribute__ ((format (printf, 1, 2)));
void            panic(char*) __attribute__((noreturn));
void            printkinit(void);

// spinlock.c
void            push_off(void);
void            pop_off(void);
int             atomic_read4(int *addr);
#ifdef LAB_LOCK
void            initrwlock(struct rwspinlock*);
void            read_acquire(struct rwspinlock*);
void            read_release(struct rwspinlock*);
void            write_acquire(struct rwspinlock*);
void            write_release(struct rwspinlock*);
uint64          sys_rwlktest(void);
#endif

// sleeplock.c

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// syscall.c
void            argint(int, int*);
int             argstr(int, char*, int);
void            argaddr(int, uint64 *);
int             fetchstr(uint64, char*, int);
int             fetchaddr(uint64, uint64*);
void            syscall();

// trap.c
extern uint     ticks;
void            trapinit(void);
void            trapinithart(void);
void            prepare_return(void);

// vm.c
void            kvminit(void);
void            kvminithart(void);
void            kvmmap(pagetable_t, uint64, uint64, uint64, int);
int             mappages(pagetable_t, uint64, uint64, uint64, int);
pagetable_t     uvmcreate(void);
uint64          uvmalloc(pagetable_t, uint64, uint64, int);
uint64          uvmdealloc(pagetable_t, uint64, uint64);
int             uvmcopy(pagetable_t, pagetable_t, uint64);
void            uvmfree(pagetable_t, uint64);
void            uvmunmap(pagetable_t, uint64, uint64, int);
void            uvmclear(pagetable_t, uint64);
pte_t *         walk(pagetable_t, uint64, int);
uint64          walkaddr(pagetable_t, uint64);
int             copyout(pagetable_t, uint64, uint64, const char *, uint64);
int             copyin(pagetable_t, uint64, char *, uint64, uint64);
int             copyinstr(pagetable_t, uint64, char *, uint64, uint64);
int             ismapped(pagetable_t, uint64);
uint64          vmfault(pagetable_t, uint64, uint64, int);
#if defined(LAB_PGTBL) || defined(SOL_MMAP)
void            vmprint(pagetable_t);
#endif
#ifdef LAB_PGTBL
uint64          sys_ksupernpte(void);
pte_t*          pgpte(pagetable_t, uint64);
#endif

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))



#ifdef LAB_PGTBL
// vmcopyin.c
int             copyin_new(pagetable_t, char *, uint64, uint64);
int             copyinstr_new(pagetable_t, char *, uint64, uint64);
#endif

#ifdef LAB_LOCK
// stats.c
void            statsinit(void);
void            statsinc(void);

// sprintf.c
int             snprintf(char*, unsigned long, const char*, ...);
#endif

#ifdef KCSAN
void            kcsaninit();
#endif

#ifdef LAB_NET
// pci.c
void            pci_init();

// e1000.c
void            e1000_init(uint32 *);
void            e1000_intr(void);
int             e1000_transmit(char *, int);

// net.c
void            netinit(void);
void            net_rx(char *buf, int len);

#endif

}
