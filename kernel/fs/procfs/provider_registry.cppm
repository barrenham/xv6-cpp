module xv.fs:proc_providers;
import :buffer;

namespace xv::fs
{
template<class... Context> class ProcRegistry final
{
public:
  static constexpr unsigned Capacity=32;
  using Render=bool (*)(Context...,PageBuffer &);
  struct Entry {char name[15]{};Render render=nullptr;unsigned mode=0444;};
  bool add(const char *name,Render render,unsigned mode=0444)
  {
    if(sealed_ || !name || !render || mode&~0444U || count_==Capacity)return false;
    unsigned size=0;while(name[size] && size<15){if(name[size]=='/')return false;++size;}
    if(!size || size>14 || find(name)>=0)return false;
    if((size==1 && name[0]=='.') || (size==2 && name[0]=='.' && name[1]=='.'))return false;
    bool numeric=true;for(unsigned i=0;i<size;++i)if(name[i]<'0' || name[i]>'9')numeric=false;
    if(numeric || (size==4 && name[0]=='s' && name[1]=='e' && name[2]=='l' && name[3]=='f'))return false;
    auto &entry=entries_[count_++];for(unsigned i=0;i<size;++i)entry.name[i]=name[i];entry.mode=mode;entry.render=render;return true;
  }
  int find(const char *name) const
  {
    // Path components may fill all 14 bytes in the retained directory ABI.
    for(unsigned i=0;i<count_;++i)
    {
      unsigned n=0;while(n<14 && name[n] && name[n]==entries_[i].name[n])++n;
      if(n==14 || (!name[n] && !entries_[i].name[n]))return int(i);
    }
    return -1;
  }
  unsigned count() const {return count_;}
  const Entry &at(unsigned index) const {return entries_[index];}
  void seal(){sealed_=true;}
private:
  Entry entries_[Capacity]{};unsigned count_=0;bool sealed_=false;
};
using ProcProviderRegistry=ProcRegistry<>;
using ProcProcessRegistry=ProcRegistry<unsigned>;
}
