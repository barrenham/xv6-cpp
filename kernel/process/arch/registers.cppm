export module xv.process:registers;

export namespace xv::process
{
struct Context
{
  unsigned long ra;
  unsigned long sp;

  // callee-saved
  unsigned long s0;
  unsigned long s1;
  unsigned long s2;
  unsigned long s3;
  unsigned long s4;
  unsigned long s5;
  unsigned long s6;
  unsigned long s7;
  unsigned long s8;
  unsigned long s9;
  unsigned long s10;
  unsigned long s11;
};

struct TrapFrame
{
  /*   0 */ unsigned long kernel_satp;   // kernel page table
  /*   8 */ unsigned long kernel_sp;     // top of process's kernel stack
  /*  16 */ unsigned long kernel_trap;   // usertrap()
  /*  24 */ unsigned long epc;           // saved user program counter
  /*  32 */ unsigned long kernel_hartid; // saved kernel tp
  /*  40 */ unsigned long ra;
  /*  48 */ unsigned long sp;
  /*  56 */ unsigned long gp;
  /*  64 */ unsigned long tp;
  /*  72 */ unsigned long t0;
  /*  80 */ unsigned long t1;
  /*  88 */ unsigned long t2;
  /*  96 */ unsigned long s0;
  /* 104 */ unsigned long s1;
  /* 112 */ unsigned long a0;
  /* 120 */ unsigned long a1;
  /* 128 */ unsigned long a2;
  /* 136 */ unsigned long a3;
  /* 144 */ unsigned long a4;
  /* 152 */ unsigned long a5;
  /* 160 */ unsigned long a6;
  /* 168 */ unsigned long a7;
  /* 176 */ unsigned long s2;
  /* 184 */ unsigned long s3;
  /* 192 */ unsigned long s4;
  /* 200 */ unsigned long s5;
  /* 208 */ unsigned long s6;
  /* 216 */ unsigned long s7;
  /* 224 */ unsigned long s8;
  /* 232 */ unsigned long s9;
  /* 240 */ unsigned long s10;
  /* 248 */ unsigned long s11;
  /* 256 */ unsigned long t3;
  /* 264 */ unsigned long t4;
  /* 272 */ unsigned long t5;
  /* 280 */ unsigned long t6;
};

static_assert(sizeof(Context) == 112);
static_assert(__builtin_offsetof(Context, sp) == 8);
static_assert(__builtin_offsetof(Context, s11) == 104);
static_assert(sizeof(TrapFrame) == 288);
static_assert(__builtin_offsetof(TrapFrame, epc) == 24);
static_assert(__builtin_offsetof(TrapFrame, a0) == 112);
static_assert(__builtin_offsetof(TrapFrame, t6) == 280);
extern "C" void swtch(Context *old_context, Context *new_context);
} // namespace xv::process
