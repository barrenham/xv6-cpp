#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "kernel/net/control.h"
#include "native_wait.h"
import xv.sync;
import xv.net;
extern "C" void net_test_device(bool, bool);
extern "C" int net_test_pages();
namespace
{
void check(bool good, const char *message)
{ if (!good) { std::fprintf(stderr,"network control FAIL: %s\n",message); std::exit(1); } }
netctl_request request(unsigned interface = 2)
{ netctl_request r{}; r.version = NETCTL_VERSION; r.size = sizeof(r); r.ifindex = interface; return r; }
int address(unsigned operation, unsigned value, unsigned prefix = 24)
{ auto r = request(); r.address = value; r.prefix = prefix; return xv::net::control(operation,r); }
void guard_contract()
{
  using namespace xv::net;
  check(xv_test_interrupt_depth() == 0, "guard starts outside interrupt critical section");
  unsigned observed = xv_test_pinned_cpu_lookups();
  std::atomic<bool> entered{false};
  std::thread contender;
  {
    ConfigurationGuard owner;
    contender = std::thread([&] {
      xv_test_require_pinned_cpu(true);
      {
        ConfigurationGuard guard;
        check(configuration_lock().holding(), "contender owns configuration lock");
        entered = true;
      }
      check(xv_test_interrupt_depth() == 0, "contender restores interrupt depth");
      xv_test_require_pinned_cpu(false);
    });
    while (xv_test_pinned_cpu_lookups() == observed) std::this_thread::yield();
    check(!entered.load(), "ownership lookup is pinned while another CPU owns the lock");
  }
  contender.join();
  check(entered.load() && xv_test_interrupt_depth() == 0, "contender acquires after outer scope releases");
  {
    ConfigurationGuard outer;
    int depth = xv_test_interrupt_depth();
    check(depth > 0 && configuration_lock().holding(), "outer guard pins CPU and owns lock");
    {
      ConfigurationGuard inner;
      check(configuration_lock().holding(), "nested guard borrows lock");
      check(interfaces().find(1), "nested guarded interface lookup");
    }
    check(configuration_lock().holding() && xv_test_interrupt_depth() == depth, "nested guard preserves outer ownership and depth");
  }
  check(xv_test_interrupt_depth() == 0, "outer guard restores entry depth");
  xv::sync::push_interrupts_off();
  int depth = xv_test_interrupt_depth();
  {
    ConfigurationGuard guard;
    check(configuration_lock().holding(), "guard entered with interrupts already disabled");
  }
  check(xv_test_interrupt_depth() == depth, "guard preserves caller interrupt disable scope");
  xv::sync::pop_interrupts_off();
  configuration_lock().acquire();
  depth = xv_test_interrupt_depth();
  {
    ConfigurationGuard borrowed;
    check(configuration_lock().holding(), "guard borrows manually acquired lock");
  }
  check(configuration_lock().holding() && xv_test_interrupt_depth() == depth, "borrowed guard does not release caller lock");
  configuration_lock().release();
  auto early_return = [] { ConfigurationGuard guard; return; };
  early_return();
  check(xv_test_interrupt_depth() == 0, "early return restores interrupt depth");
}

}
extern "C" void run_network_control_contracts()
{
  using namespace xv::net;
  guard_contract();
  auto r = request(); r.version = 2;
  check(control(NETCTL_LINK,r)==-22,"ABI version rejected");
  r=request(); r.size=0; check(control(NETCTL_LINK,r)==-22,"ABI size rejected");
  r=request(); check(control(NETCTL_LINK,r)==1 && r.mtu==1500 && (r.flags&NET_LINK_MAC),"link snapshot");
  r=request(77); check(control(NETCTL_SET_LINK,r)==-19,"unknown interface rejected");
  r=request(1); check(control(NETCTL_SET_LINK,r)==-95,"loopback remains available");
  r=request(); r.address=0xc0000200; r.prefix=24; r.source=0x7f000001;
  check(control(NETCTL_ADD_ROUTE,r)==-22,"Ethernet route cannot borrow unrelated source address");
  check(address(NETCTL_ADD_ADDRESS,0x7f000002)==-17,"loopback prefix ownership protected");
  check(address(NETCTL_ADD_ADDRESS,0x0a000210)==0,"runtime address add");
  RouteSelection selection;
  check(routing_table().lookup(0x0a000210,selection) && selection.interface->id()==1 && selection.source==0x0a000210,
        "automatic local host route");
  check(address(NETCTL_ADD_ADDRESS,0x0a000210)==-17,"duplicate alias rejected");
  r=request(1); r.address=0x0a000210; r.prefix=32; r.source=0x0a000210;
  check(control(NETCTL_DEL_ROUTE,r)==-16,"automatic route owned by address");
  std::atomic<bool> started{false}, stop{false};
  std::thread reader([&] {
    started=true;
    while (!stop.load())
    {
      auto snapshot=request(); check(control(NETCTL_LINK,snapshot)==1 && snapshot.address_count>=1 && snapshot.address_count<=4,
                                    "concurrent link snapshot");
      RouteSelection route;
      check(routing_table().lookup(0x0a000202,route) && route.source==0x0a00020f,"concurrent route snapshot");
    }
  });
  while (!started.load()) std::this_thread::yield();
  for (unsigned i=0;i<300;++i)
  {
    check(address(NETCTL_DEL_ADDRESS,0x0a000210)==0,"concurrent remove");
    check(address(NETCTL_ADD_ADDRESS,0x0a000210)==0,"concurrent add");
  }
  stop=true; reader.join();
  r=request(); r.address=0xcb007100; r.prefix=24; r.gateway=0x0a000202; r.source=0x0a000210; r.metric=7;
  check(control(NETCTL_ADD_ROUTE,r)==0 && control(NETCTL_ADD_ROUTE,r)==-17,"static route add and duplicate");
  check(routing_table().lookup(0xcb007101,selection) && selection.source==0x0a000210,"preferred route source");
  check(address(NETCTL_DEL_ADDRESS,0x0a000210)==0,"alias removal");
  check(!routing_table().lookup(0xcb007101,selection,0x0a000210),"deleted source cannot be selected");
  check(control(NETCTL_DEL_ROUTE,r)==-2,"source-dependent route removed");
  unsigned count=0;
  for (;;++count) { auto item=request(); item.index=count; if (!control(NETCTL_ROUTE,item)) break; }
  for (unsigned i=count;i<32;++i)
  { auto item=request(); item.address=0xc0000200+(i<<8); item.prefix=24; check(control(NETCTL_ADD_ROUTE,item)==0,"fill route table"); }
  r=request(); r.index=0; check(control(NETCTL_ADDRESS,r)==1 && r.address_count==0,"address query isolated fields");
  check(address(NETCTL_ADD_ADDRESS,0x0a000211)==-105,"address/route transaction capacity rollback");
  r=request(); check(control(NETCTL_LINK,r)==1 && r.address_count==1,"failed add preserves addresses");
  for (unsigned i=count;i<32;++i)
  { auto item=request(); item.address=0xc0000200+(i<<8); item.prefix=24; check(control(NETCTL_DEL_ROUTE,item)==0,"restore route table"); }
  for (unsigned i=0;i<3;++i) check(address(NETCTL_ADD_ADDRESS,0x0a000211+i)==0,"fill address table");
  check(address(NETCTL_ADD_ADDRESS,0x0a000214)==-105,"address capacity limit");
  for (unsigned i=0;i<3;++i) check(address(NETCTL_DEL_ADDRESS,0x0a000211+i)==0,"restore aliases");
  int baseline = net_test_pages();
  auto *socket = create_socket(2, 17); check(socket, "ARP cancellation socket");
  net_test_device(true, false);
  Endpoint peer{0x0a000263, 9000};
  check(socket->send_to({reinterpret_cast<unsigned long>("queued"), true}, 6, &peer)==6 && net_test_pages()==baseline+2,
        "ARP retains one pending page");
  r=request(); r.flags=0; check(control(NETCTL_SET_LINK,r)==0,"disable physical interface");
  check(net_test_pages()==baseline+1,"interface change releases queued ARP page");
  socket->close(); net_test_device(true, true);
  check(net_test_pages()==baseline,"cancelled ARP owns no leaked pages");
  check(!routing_table().lookup(0x0a000202,selection),"disabled interface removed from route selection");
  r=request(); r.flags=NET_LINK_UP; check(control(NETCTL_SET_LINK,r)==0,"restore physical interface");
  check(routing_table().lookup(0x0a000202,selection),"route resumes after enable");
  check(address(NETCTL_DEL_ADDRESS,0x0a00020f)==0,"remove final IPv4 address");
  r=request(); check(control(NETCTL_LINK,r)==1 && !r.address_count && (r.flags&NET_LINK_MAC),"physical device remains without IPv4");
  check(!routing_table().lookup(0x0a000202,selection),"addressless gateway route inactive");
  check(address(NETCTL_ADD_ADDRESS,0x0a00020f)==0 && routing_table().lookup(0x0a000202,selection),"restore final IPv4 address");
  std::puts("network control: ABI, transactions and concurrent configuration passed");
}
