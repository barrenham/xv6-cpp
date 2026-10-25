module xv.process:poll_entry;
namespace xv::process
{
class PollWait;
struct PollEntry
{
  void *channel = nullptr;
  PollWait *owner = nullptr;
  PollEntry *next = nullptr;
};
}
