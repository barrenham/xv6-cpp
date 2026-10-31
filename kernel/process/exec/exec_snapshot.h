#pragma once
#include "kernel/support/placement_new.hpp"

namespace xv::exec
{
// Immutable exec strings, shared by fork. No pointers into user memory survive.
class Snapshot final
{
public:
  enum class Field {CommandLine,Environment,Executable};
  using Allocate=void *(*)();
  using Free=void (*)(void *);
  using Sink=bool (*)(void *,const void *,unsigned);
  constexpr Snapshot()=default;
  ~Snapshot(){reset();}
  Snapshot(const Snapshot&)=delete;
  Snapshot &operator=(const Snapshot&)=delete;
  void swap(Snapshot &other){auto *previous=head_;head_=other.head_;other.head_=previous;}
  void inherit(const Snapshot &source)
  {
    if(this==&source)return;
    reset();head_=source.head_;if(head_)__atomic_add_fetch(&head_->references,1U,__ATOMIC_RELAXED);
  }
  void reset()
  {
    auto *page=head_;head_=nullptr;
    if(!page || __atomic_sub_fetch(&page->references,1U,__ATOMIC_ACQ_REL))return;
    auto release=page->release;
    while(page){auto *next=page->next;page->~Page();release(page);page=next;}
  }
  bool prepare(const char *path,char *const *arguments,char *const *environment,
               Allocate allocate,Free release,unsigned max_arguments=32,unsigned max_environment=32)
  {
    if(!path || !allocate || !release)return false;
    Snapshot candidate;Page *tail=nullptr;
    unsigned path_size=0,args_size=0,env_size=0,total=0;
    if(!candidate.string(path,allocate,release,tail,path_size,total))return false;
    for(unsigned i=0;arguments && arguments[i];++i)
    {if(i>=max_arguments || !candidate.string(arguments[i],allocate,release,tail,args_size,total))return false;}
    for(unsigned i=0;environment && environment[i];++i)
    {if(i>=max_environment || !candidate.string(environment[i],allocate,release,tail,env_size,total))return false;}
    candidate.head_->path=path_size;candidate.head_->arguments=args_size;candidate.head_->environment=env_size;
    swap(candidate);return true;
  }
  bool copy(Field field,void *context,Sink sink) const
  {
    if(!head_)return true;
    unsigned offset=0,count=0;
    switch(field)
    {
    case Field::Executable:count=head_->path ? head_->path-1:0;break;
    case Field::CommandLine:offset=head_->path;count=head_->arguments;break;
    case Field::Environment:offset=head_->path+head_->arguments;count=head_->environment;break;
    }
    for(auto *page=head_;page && count;page=page->next)
    {
      if(offset>=page->used){offset-=page->used;continue;}
      unsigned take=page->used-offset;if(take>count)take=count;
      if(!sink(context,page->bytes+offset,take))return false;
      offset=0;count-=take;
    }
    return count==0;
  }
private:
  struct Page
  {
    Page *next;Free release;
    unsigned references,path,arguments,environment,used;
    unsigned char bytes[4096-2*sizeof(void *)-5*sizeof(unsigned)];
    explicit Page(Free free):next(nullptr),release(free),references(1),path(0),arguments(0),environment(0),used(0) {}
  };
  static_assert(sizeof(Page)==4096);
  Page *head_=nullptr;
  bool string(const char *text,Allocate allocate,Free release,Page *&tail,unsigned &size,unsigned &total)
  {
    constexpr unsigned Limit=262144;
    unsigned length=0;
    do{if(length==Limit-total)return false;++length;}while(text[length-1]);
    unsigned copied=0;
    while(copied<length)
    {
      if(!tail || tail->used==sizeof(tail->bytes))
      {
        void *memory=allocate();if(!memory)return false;
        auto *page=new(memory) Page(release);if(tail)tail->next=page;else head_=page;tail=page;
      }
      unsigned take=sizeof(tail->bytes)-tail->used;if(take>length-copied)take=length-copied;
      for(unsigned i=0;i<take;++i)tail->bytes[tail->used+i]=static_cast<unsigned char>(text[copied+i]);
      tail->used+=take;copied+=take;
    }
    size+=length;total+=length;return true;
  }
};
}
