export module xv.sync:interrupts;

export namespace xv::sync
{
// Matched calls preserve the caller's original interrupt-enable state.
void push_interrupts_off();
void pop_interrupts_off();
unsigned interrupt_depth();
} // namespace xv::sync
