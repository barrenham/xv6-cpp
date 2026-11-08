#pragma once
#include "kmodule_v3.h"

// ABI 4 adds an explicit quiesce/remove contract; older resident ELFs remain valid.
#define XV_KMOD_ABI_V4 4U
#define XV_KMOD_RESCAN 7U
#define XV_KMOD_HOTPLUG_STATUS 8U
#define XV_KMOD_REMOVE 9U
#define XV_KMOD_EVENT 10U
#define XV_KMOD_PRESENT 1U
#define XV_KMOD_REMOVABLE 2U
#define XV_KMOD_BOUND 4U
#define XV_KMOD_REMOVE_PENDING 8U
#define XV_KMOD_REMOVE_FAILED 16U
#define XV_KMOD_CAN_REMOVE 32U
#define XV_KMOD_EVENT_ADDED 1U
#define XV_KMOD_EVENT_BOUND 2U
#define XV_KMOD_EVENT_REMOVED 3U
#define XV_KMOD_EVENT_REJECTED 4U
#define XV_KMOD_EVENT_FAILED 5U
#define XV_KMOD_REMOVE_ORDERLY 0U
#define XV_KMOD_REMOVE_SURPRISE 1U

struct xv_kmod_driver_v4
{
  struct xv_kmod_driver_v3 base;
  // Called only after the host blocks new calls and drains in-flight callbacks.
  // Success: stop DMA and release this binding's resources, including its state.
  // Failure: retain all resources; the host keeps the binding quarantined.
  int (*remove)(const struct xv_kmod_host_v2 *, const struct xv_kmod_device_v3 *,
                unsigned reason);
};

struct xv_kmod_hotplug_status
{
  unsigned size, index, generation, flags;
  int last_error;
  unsigned network_slot;
  struct xv_kmod_device identity;
};

struct xv_kmod_event
{
  unsigned size, type, index, generation;
  unsigned long sequence, lost;
  char name[XV_KMOD_NAME];
};
