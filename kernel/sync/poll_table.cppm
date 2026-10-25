export module xv.sync:poll;

export namespace xv::sync
{
// Device poll methods register queues before testing readiness. watch() never
// sleeps and may run with the object's condition lock held.
class PollTable
{
public:
  virtual void watch(void *channel) = 0;
  unsigned events() const { return events_; }
  void request(unsigned events) { events_ = events; }
protected:
  constexpr PollTable() = default;
  ~PollTable() = default;
private:
  unsigned events_ = 0;
};
}
