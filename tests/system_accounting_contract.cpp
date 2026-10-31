#include "kernel/support/system_accounting.h"
#include <cassert>
#include <cstdio>
#include <thread>
int main()
{
  xv::stats::Accounting<4> stats;xv::stats::Cpu cpu;
  assert(!stats.cpu(4,cpu));stats.timer(4,true,true);stats.external(9);
  assert(stats.system().switches==0 && stats.system().processes==0);
  stats.timer(0,true,true);stats.timer(0,false,true);stats.timer(0,false,false);stats.external(0);
  assert(stats.cpu(0,cpu) && cpu.user==1 && cpu.system==1 && cpu.idle==1 && cpu.interrupts==4);
  assert(stats.cpu(1,cpu) && cpu.interrupts==0);
  std::thread workers[8];
  for(unsigned thread=0;thread<8;++thread)workers[thread]=std::thread([&,thread]{
    for(unsigned i=0;i<10000;++i)
    {stats.timer(thread%4,true,true);stats.timer(thread%4,false,true);stats.timer(thread%4,false,false);stats.external(thread%4);stats.dispatch();stats.created();}
  });
  for(auto &worker:workers)worker.join();
  auto system=stats.system();assert(system.switches==80000 && system.processes==80000);
  for(unsigned i=0;i<4;++i)
  {
    assert(stats.cpu(i,cpu));auto initial=i==0 ? 1UL:0UL;
    assert(cpu.user==20000+initial && cpu.system==20000+initial && cpu.idle==20000+initial && cpu.interrupts==80000+4*initial);
  }
  std::puts("system accounting: categories, CPU bounds, independent samples and concurrent writers passed");
}
