module;
#include "kernel/net/errno.h"
module xv.net;
import :lease;
namespace xv::net
{
int LeaseConfiguration::install(NetworkInterface &interface,const DhcpLease &lease,bool &changed)
{
  ConfigurationGuard guard; changed=false;
  if (!lease.address || lease.prefix>32 || !lease.seconds) return -EINVAL;
  if (interfaces_.find(interface.id())!=&interface || !interface.up() || interface.master()) return -ENETUNREACH;
  unsigned token=interface.id(), position=interface.address_count_, old_routes=0;
  for (unsigned i=0;i<interface.address_count_;++i)
    if (interface.addresses_[i].owner==token) position=i;
  for (unsigned i=0;i<interfaces_.count();++i)
  {
    auto *other=interfaces_.at(i);
    if (other!=&interface && other->owns(lease.address)) return -EADDRINUSE;
  }
  for (unsigned i=0;i<interface.address_count_;++i)
    if (i!=position && interface.addresses_[i].address==lease.address) return -EADDRINUSE;
  unsigned needed=lease.gateway ? 3 : 2;
  for (unsigned i=0;i<routes_.count_;++i) if (routes_.routes_[i].lease_owner==token) ++old_routes;
  if ((position==interface.address_count_ && position==4) || routes_.count_-old_routes+needed>32) return -ENOBUFS;
  RoutingTable::Route desired[3] = {
    {lease.address,32,0,0,lease.address,&loopback_,interface.id(),lease.address,token},
    {lease.address&Ipv4Address{0,lease.prefix}.mask(),lease.prefix,0,0,0,&interface,interface.id(),lease.address,token},
    {0,0,lease.gateway,100+interface.id(),lease.address,&interface,0,0,token}};
  bool same=position<interface.address_count_ && interface.addresses_[position].address==lease.address &&
            interface.addresses_[position].prefix==lease.prefix && old_routes==needed;
  for (unsigned n=0;n<needed && same;++n)
  {
    bool found=false;
    for (unsigned i=0;i<routes_.count_;++i)
    {
      const auto &r=routes_.routes_[i], &d=desired[n];
      if (r.lease_owner==token && r.interface==d.interface && r.network==d.network && r.prefix==d.prefix &&
          r.source==d.source && r.gateway==d.gateway && r.metric==d.metric) found=true;
    }
    same=found;
  }
  if (same) return 0;
  unsigned count=0;
  for (unsigned i=0;i<routes_.count_;++i)
    if (routes_.routes_[i].lease_owner!=token) routes_.routes_[count++]=routes_.routes_[i];
  for (unsigned i=count;i<routes_.count_;++i) routes_.routes_[i]={};
  routes_.count_=count;
  if (position==interface.address_count_) ++interface.address_count_;
  interface.addresses_[position]={lease.address,lease.prefix,token};
  for (unsigned n=0;n<needed;++n) routes_.routes_[routes_.count_++]=desired[n];
  ++interface.generation_; changed=true; return 0;
}
bool LeaseConfiguration::remove(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  unsigned token=interface.id(), count=0; bool changed=false;
  for (unsigned i=0;i<interface.address_count_;++i)
    if (interface.addresses_[i].owner!=token) interface.addresses_[count++]=interface.addresses_[i];
    else changed=true;
  for (unsigned i=count;i<interface.address_count_;++i) interface.addresses_[i]={};
  interface.address_count_=count; count=0;
  for (unsigned i=0;i<routes_.count_;++i)
    if (routes_.routes_[i].lease_owner!=token) routes_.routes_[count++]=routes_.routes_[i];
    else changed=true;
  for (unsigned i=count;i<routes_.count_;++i) routes_.routes_[i]={};
  routes_.count_=count;
  if (changed) ++interface.generation_;
  return changed;
}
}
