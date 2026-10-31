#pragma once
namespace xv::stats
{
struct Cpu {unsigned long user=0,system=0,idle=0,interrupts=0;};
struct System {unsigned long switches=0,processes=0;};
// Timer samples are 100 ms. Each counter is independently monotonic; a
// snapshot does not stop other CPUs and is not a globally atomic instant.
template<unsigned CpuCount> class Accounting final
{
public:
  constexpr Accounting()=default;
  void timer(unsigned cpu,bool user,bool active)
  {
    if(cpu>=CpuCount)return;
    auto &record=cpus_[cpu];
    add(user ? record.user:active ? record.system:record.idle);
    add(record.interrupts);
  }
  void external(unsigned cpu){if(cpu<CpuCount)add(cpus_[cpu].interrupts);}
  void dispatch(){add(system_.switches);}
  void created(){add(system_.processes);}
  bool cpu(unsigned index,Cpu &result) const
  {
    if(index>=CpuCount)return false;
    const auto &source=cpus_[index];
    result={get(source.user),get(source.system),get(source.idle),get(source.interrupts)};return true;
  }
  System system() const {return {get(system_.switches),get(system_.processes)};}
private:
  Cpu cpus_[CpuCount]{};
  System system_{};
  static void add(unsigned long &value){__atomic_fetch_add(&value,1UL,__ATOMIC_RELAXED);}
  static unsigned long get(const unsigned long &value)
  {unsigned long result;__atomic_load(&value,&result,__ATOMIC_RELAXED);return result;}
};
}
