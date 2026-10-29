export module xv.memory;

// Public API is listed here; concrete allocator implementations stay internal.
export import :address;
export import :range;
export import :layout;
export import xv.vm_pool;
export import xv.vm_types;
export import :boot;
export import :allocator;
export import :sv39;
export import :kernel_paging;
export import :user_paging;
