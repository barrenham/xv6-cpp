module xv.boot:state;

namespace xv::boot::detail
{
// Only the bootstrap hart consumes the firmware device-tree address.
unsigned long device_tree();
} // namespace xv::boot::detail
