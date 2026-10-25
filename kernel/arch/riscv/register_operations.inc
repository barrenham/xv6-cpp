// Shared instruction definitions; include inside the desired namespace.
// which hart (core) is this?
inline uint64
r_mhartid()
{
  uint64 x;
  asm volatile("csrr %0, mhartid" : "=r"(x));
  return x;
}

// Machine Status Register, mstatus


inline uint64
r_mstatus()
{
  uint64 x;
  asm volatile("csrr %0, mstatus" : "=r"(x));
  return x;
}

inline void
w_mstatus(uint64 x)
{
  asm volatile("csrw mstatus, %0" : : "r"(x));
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
inline void
w_mepc(uint64 x)
{
  asm volatile("csrw mepc, %0" : : "r"(x));
}

// Supervisor Status Register, sstatus


inline uint64
r_sstatus()
{
  uint64 x;
  asm volatile("csrr %0, sstatus" : "=r"(x));
  return x;
}

inline void
w_sstatus(uint64 x)
{
  asm volatile("csrw sstatus, %0" : : "r"(x));
}

inline void
s_sstatus(uint64 x)
{
  __asm__ __volatile__("csrs sstatus, %0" ::"rK"(x) : "memory");
}

inline void
c_sstatus(uint64 x)
{
  __asm__ __volatile__("csrc sstatus, %0" ::"rK"(x) : "memory");
}

inline uint64
rc_sstatus(uint64 x)
{
  __asm__ __volatile__("csrrc %0, sstatus, %1" : "=r"(x) : "rK"(x) : "memory");
  return x;
}

// Supervisor Interrupt Pending
inline uint64
r_sip()
{
  uint64 x;
  asm volatile("csrr %0, sip" : "=r"(x));
  return x;
}

inline void
w_sip(uint64 x)
{
  asm volatile("csrw sip, %0" : : "r"(x));
}

// Supervisor Interrupt Enable
inline uint64
r_sie()
{
  uint64 x;
  asm volatile("csrr %0, sie" : "=r"(x));
  return x;
}

inline void
w_sie(uint64 x)
{
  asm volatile("csrw sie, %0" : : "r"(x));
}

// Machine-mode Interrupt Enable
inline uint64
r_mie()
{
  uint64 x;
  asm volatile("csrr %0, mie" : "=r"(x));
  return x;
}

inline void
w_mie(uint64 x)
{
  asm volatile("csrw mie, %0" : : "r"(x));
}

// supervisor exception program counter, holds the
// instruction address to which a return from
// exception will go.
inline void
w_sepc(uint64 x)
{
  asm volatile("csrw sepc, %0" : : "r"(x));
}

inline uint64
r_sepc()
{
  uint64 x;
  asm volatile("csrr %0, sepc" : "=r"(x));
  return x;
}

// Machine Exception Delegation
inline uint64
r_medeleg()
{
  uint64 x;
  asm volatile("csrr %0, medeleg" : "=r"(x));
  return x;
}

inline void
w_medeleg(uint64 x)
{
  asm volatile("csrw medeleg, %0" : : "r"(x));
}

// Machine Interrupt Delegation
inline uint64
r_mideleg()
{
  uint64 x;
  asm volatile("csrr %0, mideleg" : "=r"(x));
  return x;
}

inline void
w_mideleg(uint64 x)
{
  asm volatile("csrw mideleg, %0" : : "r"(x));
}

// Supervisor Trap-Vector Base Address
// low two bits are mode.
inline void
w_stvec(uint64 x)
{
  asm volatile("csrw stvec, %0" : : "r"(x));
}

inline uint64
r_stvec()
{
  uint64 x;
  asm volatile("csrr %0, stvec" : "=r"(x));
  return x;
}

// Machine Trap-Vector Base Address
// low two bits are mode.
inline void
w_mtvec(uint64 x)
{
  asm volatile("csrw mtvec, %0" : : "r"(x));
}

inline uint64
r_mtvec()
{
  uint64 x;
  asm volatile("csrr %0, mtvec" : "=r"(x));
  return x;
}

// Supervisor Timer Comparison Register
inline uint64
r_stimecmp()
{
  uint64 x;
  // asm volatile("csrr %0, stimecmp" : "=r" (x) );
  asm volatile("csrr %0, 0x14d" : "=r"(x));
  return x;
}

inline void
w_stimecmp(uint64 x)
{
  // asm volatile("csrw stimecmp, %0" : : "r" (x));
  asm volatile("csrw 0x14d, %0" : : "r"(x));
}

// Machine Environment Configuration Register


inline uint64
r_menvcfg()
{
  uint64 x;
  // asm volatile("csrr %0, menvcfg" : "=r" (x) );
  asm volatile("csrr %0, 0x30a" : "=r"(x));
  return x;
}

inline void
w_menvcfg(uint64 x)
{
  //asm volatile("csrw menvcfg, %0" : : "r" (x));
  asm volatile("csrw 0x30a, %0" : : "r"(x));
}

// Physical Memory Protection
inline void
w_pmpcfg0(uint64 x)
{
  asm volatile("csrw pmpcfg0, %0" : : "r"(x));
}

inline void
w_pmpaddr0(uint64 x)
{
  asm volatile("csrw pmpaddr0, %0" : : "r"(x));
}

// use riscv's sv39 page table scheme.


// supervisor address translation and protection;
// holds the address of the page table.
inline void
w_satp(uint64 x)
{
  asm volatile("csrw satp, %0" : : "r"(x));
}

inline uint64
r_satp()
{
  uint64 x;
  asm volatile("csrr %0, satp" : "=r"(x));
  return x;
}

// Supervisor Trap Cause
inline uint64
r_scause()
{
  uint64 x;
  asm volatile("csrr %0, scause" : "=r"(x));
  return x;
}

// Supervisor Trap Value
inline uint64
r_stval()
{
  uint64 x;
  asm volatile("csrr %0, stval" : "=r"(x));
  return x;
}

// Machine-mode Counter-Enable
inline void
w_mcounteren(uint64 x)
{
  asm volatile("csrw mcounteren, %0" : : "r"(x));
}

inline uint64
r_mcounteren()
{
  uint64 x;
  asm volatile("csrr %0, mcounteren" : "=r"(x));
  return x;
}

// machine-mode cycle counter
inline uint64
r_time()
{
  uint64 x;
  asm volatile("csrr %0, time" : "=r"(x));
  return x;
}

// enable device interrupts
inline void
intr_on()
{
  s_sstatus(SSTATUS_SIE);
}

// disable device interrupts
inline void
intr_off()
{
  c_sstatus(SSTATUS_SIE);
}

// are device interrupts enabled?
inline int
intr_get()
{
  uint64 x = r_sstatus();
  return (x & SSTATUS_SIE) != 0;
}

inline uint64
r_sp()
{
  uint64 x;
  asm volatile("mv %0, sp" : "=r"(x));
  return x;
}

inline uint64
r_fp()
{
  uint64 x;
  asm volatile("mv %0, s0" : "=r"(x));
  return x;
}

// read and write tp, the thread pointer, which xv6 uses to hold
// this core's hartid (core number), the index into cpus[].
inline uint64
r_tp()
{
  uint64 x;
  asm volatile("mv %0, tp" : "=r"(x));
  return x;
}

inline void
w_tp(uint64 x)
{
  asm volatile("mv tp, %0" : : "r"(x));
}

inline uint64
r_ra()
{
  uint64 x;
  asm volatile("mv %0, ra" : "=r"(x));
  return x;
}

// flush the TLB.
inline void
sfence_vma()
{
  // the zero, zero means flush all TLB entries.
  asm volatile("sfence.vma zero, zero" ::: "memory");
}

// fence for memory-mapped IO
inline void
io_fence()
{
  asm volatile("fence iorw, iorw" ::: "memory");
}

// fence for icache
inline void
icache_fence()
{
  asm volatile("fence.i" ::: "memory");
}

