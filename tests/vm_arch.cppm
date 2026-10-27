export module xv.arch;
export namespace xv::arch
{
void sfence_vma() { __atomic_thread_fence(__ATOMIC_SEQ_CST); }
}
