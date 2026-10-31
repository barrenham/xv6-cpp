module;
#include "kernel/support/flow.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"
module xv.drivers;
import :entropy;
import :platform;
namespace xv::drivers
{
namespace flow=xv::flow;
struct VirtioEntropy::InitializeFlow
{
  struct Context {VirtioEntropy &owner;unsigned status=0;bool found=false,okay=false;};
  static void identify(Context &c) noexcept
  {
    c.owner.lock_.init("virtio entropy");
    auto &r=c.owner.registers_;
    c.found=r.read32(VIRTIO_MMIO_MAGIC_VALUE)==0x74726976 &&
      r.read32(VIRTIO_MMIO_VERSION)==2 && r.read32(VIRTIO_MMIO_DEVICE_ID)==4;
  }
  static bool found(Context &c) noexcept {return c.found;}
  static void negotiate(Context &c) noexcept
  {
    auto &r=c.owner.registers_;
    r.write32(VIRTIO_MMIO_STATUS,0);
    r.write32(VIRTIO_MMIO_STATUS,VIRTIO_CONFIG_S_ACKNOWLEDGE);
    c.status=VIRTIO_CONFIG_S_ACKNOWLEDGE|VIRTIO_CONFIG_S_DRIVER;
    r.write32(VIRTIO_MMIO_STATUS,c.status);
    r.write32(VIRTIO_MMIO_DEVICE_FEATURES_SEL,1);
    if(!(r.read32(VIRTIO_MMIO_DEVICE_FEATURES)&1))return;
    r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL,0);r.write32(VIRTIO_MMIO_DRIVER_FEATURES,0);
    r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL,1);r.write32(VIRTIO_MMIO_DRIVER_FEATURES,1);
    r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL,0);
    c.status|=VIRTIO_CONFIG_S_FEATURES_OK;r.write32(VIRTIO_MMIO_STATUS,c.status);
    if(!(r.read32(VIRTIO_MMIO_STATUS)&VIRTIO_CONFIG_S_FEATURES_OK))return;
    r.write32(VIRTIO_MMIO_QUEUE_SEL,0);
    c.okay=!r.read32(VIRTIO_MMIO_QUEUE_READY) && r.read32(VIRTIO_MMIO_QUEUE_NUM_MAX)>=QueueSize;
  }
  static bool okay(Context &c) noexcept {return c.okay;}
  static void allocate(Context &c) noexcept
  {
    c.owner.descriptors_=static_cast<virtq_desc *>(detail::allocate_dma_page());
    c.owner.available_=static_cast<virtq_avail *>(detail::allocate_dma_page());
    c.owner.used_=static_cast<virtq_used *>(detail::allocate_dma_page());
    c.owner.bytes_=static_cast<unsigned char *>(detail::allocate_dma_page());
    c.okay=c.owner.descriptors_ && c.owner.available_ && c.owner.used_ && c.owner.bytes_;
  }
  static void fail(Context &c) noexcept
  {
    if(c.owner.descriptors_)detail::free_dma_page(c.owner.descriptors_);
    if(c.owner.available_)detail::free_dma_page(c.owner.available_);
    if(c.owner.used_)detail::free_dma_page(const_cast<virtq_used *>(c.owner.used_));
    if(c.owner.bytes_)detail::free_dma_page(c.owner.bytes_);
    c.owner.descriptors_=nullptr;c.owner.available_=nullptr;c.owner.used_=nullptr;c.owner.bytes_=nullptr;
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS,c.status|128U);
  }
  static void publish(Context &c) noexcept
  {
    detail::zero_page(c.owner.descriptors_);detail::zero_page(c.owner.available_);
    detail::zero_page(const_cast<virtq_used *>(c.owner.used_));detail::zero_page(c.owner.bytes_);
    auto &r=c.owner.registers_;
    r.write32(VIRTIO_MMIO_QUEUE_NUM,QueueSize);
    auto address=[&](unsigned low,unsigned high,const volatile void *pointer)
    {auto value=reinterpret_cast<unsigned long>(pointer);r.write32(low,unsigned(value));r.write32(high,unsigned(value>>32));};
    address(VIRTIO_MMIO_QUEUE_DESC_LOW,VIRTIO_MMIO_QUEUE_DESC_HIGH,c.owner.descriptors_);
    address(VIRTIO_MMIO_DRIVER_DESC_LOW,VIRTIO_MMIO_DRIVER_DESC_HIGH,c.owner.available_);
    address(VIRTIO_MMIO_DEVICE_DESC_LOW,VIRTIO_MMIO_DEVICE_DESC_HIGH,c.owner.used_);
    detail::io_barrier();r.write32(VIRTIO_MMIO_QUEUE_READY,1);
    r.write32(VIRTIO_MMIO_STATUS,c.status|VIRTIO_CONFIG_S_DRIVER_OK);
    c.owner.ready_=true;c.owner.request();
  }
  struct Identify;struct Negotiate;struct Allocate;struct Fail;struct Publish;
  using Graph=flow::graph<Identify,flow::types<
    flow::node<Identify,flow::action<&identify>,flow::branch<flow::condition<&found>,flow::go<Negotiate>,flow::stop>>,
    flow::node<Negotiate,flow::action<&negotiate>,flow::branch<flow::condition<&okay>,flow::go<Allocate>,flow::go<Fail>>>,
    flow::node<Allocate,flow::action<&allocate>,flow::branch<flow::condition<&okay>,flow::go<Publish>,flow::go<Fail>>>,
    flow::node<Fail,flow::action<&fail>,flow::stop>,flow::node<Publish,flow::action<&publish>,flow::stop>>>;
};
struct VirtioEntropy::ReadFlow
{
  struct Context {VirtioEntropy &owner;IoBuffer destination;int count,result=-1;};
  static bool available(Context &c) noexcept {return c.owner.read_<c.owner.size_;}
  static bool stopped(Context &c) noexcept {return !c.owner.ready_ || detail::process_killed();}
  static bool nonblocking(Context &c) noexcept {return c.destination.nonblocking;}
  static void again(Context &c) noexcept {c.result=-EAGAIN;}
  static void sleep(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.size_);c.owner.lock_.release();
    detail::sleep_process();c.owner.lock_.acquire();
  }
  static void copy(Context &c) noexcept
  {
    unsigned take=c.owner.size_-c.owner.read_;
    if(take>unsigned(c.count))take=c.count;
    if(take>256)take=256;
    if(detail::copy_out(c.destination.space==AddressSpace::User,c.destination.address,c.owner.bytes_+c.owner.read_,take)<0)return;
    c.result=take;c.owner.read_+=take;
    if(c.owner.read_==c.owner.size_){c.owner.read_=c.owner.size_=0;c.owner.request();}
  }
  struct Check;struct Available;struct Nonblocking;struct Again;struct Sleep;struct Copy;
  using Graph=flow::graph<Check,flow::types<
    flow::node<Check,flow::noop,flow::branch<flow::condition<&stopped>,flow::stop,flow::go<Available>>>,
    flow::node<Available,flow::noop,flow::branch<flow::condition<&available>,flow::go<Copy>,flow::go<Nonblocking>>>,
    flow::node<Nonblocking,flow::noop,flow::branch<flow::condition<&nonblocking>,flow::go<Again>,flow::go<Sleep>>>,
    flow::node<Again,flow::action<&again>,flow::stop>,flow::node<Sleep,flow::action<&sleep>,flow::go<Check>>,
    flow::node<Copy,flow::action<&copy>,flow::stop>>>;
};
void VirtioEntropy::initialize()
{InitializeFlow::Context context{*this};(void)InitializeFlow::Graph::run<0>(context);}
void VirtioEntropy::request()
{
  descriptors_[0]={reinterpret_cast<unsigned long>(bytes_),4096,VRING_DESC_F_WRITE,0};
  pending_=true;available_->ring[available_->idx%QueueSize]=0;
  detail::io_barrier();++available_->idx;detail::io_barrier();registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY,0);
}
int VirtioEntropy::read(IoBuffer destination,int count)
{
  if(count<=0)return count ? -1:0;
  xv::sync::ScopedLock guard(lock_);ReadFlow::Context context{*this,destination,count};
  (void)ReadFlow::Graph::run<0>(context);return context.result;
}
unsigned VirtioEntropy::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(lock_);if(table)table->watch(&size_);
  return !ready_ ? POLLERR:read_<size_ ? POLLIN:0;
}
void VirtioEntropy::handle_interrupt()
{
  xv::sync::ScopedLock guard(lock_);
  registers_.write32(VIRTIO_MMIO_INTERRUPT_ACK,registers_.read32(VIRTIO_MMIO_INTERRUPT_STATUS)&3);
  detail::io_barrier();if(!ready_ || used_->idx==used_index_)return;
  if(!pending_ || static_cast<unsigned short>(used_->idx-used_index_)!=1)detail::fail("entropy: unexpected completion");
  const auto &entry=used_->ring[used_index_%QueueSize];
  unsigned id=entry.id,length=entry.len;
  if(id!=0 || !length || length>4096)detail::fail("entropy: malformed completion");
  detail::io_barrier();++used_index_;pending_=false;read_=0;size_=length;
  detail::wake_processes(&size_);
}
}
