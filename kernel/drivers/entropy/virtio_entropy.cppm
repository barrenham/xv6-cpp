module xv.drivers:entropy;
import :character;
import :interrupt;
import :protocol;
import :mmio;
import xv.sync;
namespace xv::drivers
{
class VirtioEntropy final : public CharacterDevice,public InterruptDevice
{
public:
  constexpr VirtioEntropy(unsigned long base,unsigned irq):registers_(base),irq_(irq) {}
  const char *name() const override {return "VirtIO entropy";}
  unsigned irq() const override {return irq_;}
  void initialize() override;
  int read(IoBuffer destination,int count) override;
  int write(IoBuffer,int count) override {return count==0 ? 0:-1;}
  void put_sync(int) override {}
  unsigned poll(xv::sync::PollTable *table=nullptr) override;
  void handle_interrupt() override;
private:
  struct InitializeFlow;struct ReadFlow;
  void request();
  detail::MmioRegion registers_;
  unsigned irq_;
  xv::sync::SpinLock lock_;
  virtq_desc *descriptors_=nullptr;
  virtq_avail *available_=nullptr;
  volatile virtq_used *used_=nullptr;
  unsigned char *bytes_=nullptr;
  unsigned short used_index_=0;
  unsigned read_=0,size_=0;
  bool ready_=false,pending_=false;
};
}
