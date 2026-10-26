# `kernel/memory/paging/user_paging.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/user_paging.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:user_paging`。

## 函数导航

- [`xv::memory::allocate_user_pages`](vm.cpp.md#fn-607dd438d1bfea8d)
- [`xv::memory::clone_user_pages`](vm.cpp.md#fn-c06f7a1b31c06b45)
- [`xv::memory::copy_from_user`](vm.cpp.md#fn-702f4fdb8fe6d6ed)
- [`xv::memory::copy_to_user`](vm.cpp.md#fn-c818984f3c716ba3)
- [`xv::memory::copy_user_string`](vm.cpp.md#fn-7bd0107be1202abf)
- [`xv::memory::create_user_table`](vm.cpp.md#fn-3fcf1b0c323aaf56)
- [`xv::memory::deallocate_user_pages`](vm.cpp.md#fn-c457a06f835dd6c8)
- [`xv::memory::demote_user_pages`](vm.cpp.md#fn-e612c2153ae52e2e)
- [`xv::memory::free_user_table`](vm.cpp.md#fn-e5328f0087ac74d2)
- [`xv::memory::is_user_mapped`](vm.cpp.md#fn-d315e799cdeadf7a)
- [`xv::memory::map_kernel_range`](vm.cpp.md#fn-e6f134e225e772ad)
- [`xv::memory::map_user_pages`](vm.cpp.md#fn-d1d927ee41b35bf8)
- [`xv::memory::print_page_table`](vm.cpp.md#fn-43a58229b9ef1c31)
- [`xv::memory::protect_user_guard`](vm.cpp.md#fn-91a20f4d785cbe8a)
- [`xv::memory::resolve_user_fault`](vm.cpp.md#fn-3fa77ef7a3677ae6)
- [`xv::memory::translate_user_page`](vm.cpp.md#fn-3be6479c7074a2cd)
- [`xv::memory::unmap_user_pages`](vm.cpp.md#fn-99f83a0dbe555628)
- [`xv::memory::walk_entry`](vm.cpp.md#fn-f2c541337b31c9a9)
