#pragma once
extern "C"
{
void xv_storage_fail_after(int allocations);
int xv_storage_live_pages();
int run_process_storage_contracts();
}
