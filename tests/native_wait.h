#pragma once

extern "C" void xv_test_prepare_sleep(void *channel);
extern "C" void xv_test_sleep();
extern "C" void xv_test_wakeup(void *channel);

extern "C" void xv_test_set_interrupted(bool value);

extern "C" int xv_test_interrupt_depth();
extern "C" void xv_test_require_pinned_cpu(bool enabled);
extern "C" unsigned xv_test_pinned_cpu_lookups();
