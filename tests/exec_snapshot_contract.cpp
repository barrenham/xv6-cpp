#include "kernel/process/exec/exec_snapshot.h"
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
using xv::exec::Snapshot;
namespace
{
std::atomic<unsigned> live{0},allocations{0};int budget=-1;
void *allocate()
{
  if(!budget)return nullptr;if(budget>0)--budget;
  auto *page=std::aligned_alloc(4096,4096);if(page){++live;++allocations;}return page;
}
void release(void *page){--live;std::free(page);}
bool append(void *context,const void *bytes,unsigned size)
{static_cast<std::string *>(context)->append(static_cast<const char *>(bytes),size);return true;}
std::string read(const Snapshot &snapshot,Snapshot::Field field)
{std::string result;assert(snapshot.copy(field,&result,&append));return result;}
}
int main()
{
  Snapshot root;char program[]="worker",argument[]="two words",empty[]="",variable[]="USER=owner";
  char *args[]={program,argument,empty,nullptr},*environment[]={variable,nullptr};
  assert(root.prepare("/bin/worker",args,environment,&allocate,&release));
  const auto original=read(root,Snapshot::Field::CommandLine);
  assert(original==std::string("worker\0two words\0\0",18));
  assert(read(root,Snapshot::Field::Environment)==std::string("USER=owner\0",11));
  assert(read(root,Snapshot::Field::Executable)=="/bin/worker");
  program[0]='X';variable[0]='Z';assert(read(root,Snapshot::Field::CommandLine)==original);
  Snapshot forked;forked.inherit(root);unsigned before=allocations;
  budget=0;
  {std::vector<Snapshot> children(1000);for(auto &child:children){child.inherit(root);assert(read(child,Snapshot::Field::CommandLine)==original);}}
  assert(allocations==before && live==1);
  std::string long_argument(9000,'l');char *long_args[]={long_argument.data(),nullptr};
  for(budget=0;budget<3;++budget)
  {
    int available=budget;assert(!root.prepare("/bin/large",long_args,nullptr,&allocate,&release));
    assert(read(root,Snapshot::Field::CommandLine)==original && live==1);budget=available;
  }
  budget=3;assert(root.prepare("/bin/large",long_args,nullptr,&allocate,&release));
  assert(live==4 && read(forked,Snapshot::Field::CommandLine)==original);
  forked.reset();assert(live==3);root.reset();assert(!live);budget=-1;
  for(unsigned length:{0U,4040U,4059U,4060U,8120U,9000U})
  {
    std::string text(length,'b');char *items[]={text.data(),nullptr};
    assert(root.prepare("/bin/boundary",items,nullptr,&allocate,&release));
    assert(read(root,Snapshot::Field::CommandLine)==text+std::string(1,'\0'));
    auto previous=read(root,Snapshot::Field::CommandLine);
    assert(!root.prepare(nullptr,items,nullptr,&allocate,&release));
    assert(read(root,Snapshot::Field::CommandLine)==previous);
    root.reset();assert(!live);
  }
  assert(root.prepare("/bin/threaded",args,environment,&allocate,&release));
  auto expected=read(root,Snapshot::Field::CommandLine);before=allocations;
  std::thread workers[8];
  for(auto &worker:workers)worker=std::thread([&]{for(unsigned i=0;i<1000;++i){Snapshot copy;copy.inherit(root);assert(read(copy,Snapshot::Field::CommandLine)==expected);}});
  for(auto &worker:workers)worker.join();assert(allocations==before && live==1);
  root.reset();assert(!live);
  std::puts("exec snapshot: binary strings, ownership, fork sharing, cross-page ranges, OOM rollback and concurrent references passed");
}
