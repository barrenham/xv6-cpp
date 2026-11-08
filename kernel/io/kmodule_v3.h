#pragma once
#include "kmodule_v2.h"

// Descriptor ABI 3 reuses host-services ABI 2. Hardware identities and matching
// are extended without changing the meaning/layout of existing V1/V2 records.
#define XV_KMOD_ABI_V3 3U
#define XV_KMOD_STATUS 5U
#define XV_KMOD_DEVICE_V3 6U
#define XV_KMOD_DEVICE_COMPATIBLE_TOO_LONG 2U
#define XV_KMOD_MATCH_VENDOR 1U
#define XV_KMOD_MATCH_DEVICE 2U
#define XV_KMOD_MATCH_SUBSYSTEM 4U
#define XV_KMOD_MATCH_REVISION 8U
#define XV_KMOD_MATCH_CLASS 16U
#define XV_KMOD_MATCH_COMPATIBLE 32U
#define XV_KMOD_MATCH_FLAGS 63U
#define XV_KMOD_MAX_MATCHES 32U
#define XV_KMOD_MAX_DEPENDENCIES 8U
#define XV_KMOD_COMPATIBLE_BYTES 256U
#define XV_KMOD_STATE_LOADED 0U
#define XV_KMOD_STATE_WAITING 1U
#define XV_KMOD_STATE_READY 2U
#define XV_KMOD_STATE_FAILED 3U

struct xv_kmod_device_v3
{
  struct xv_kmod_device_v2 base;
  unsigned revision, class_code, subsystem_vendor, subsystem_device;
  unsigned compatible_bytes;
  char compatible[XV_KMOD_COMPATIBLE_BYTES]; // NUL-separated DT compatible list
  unsigned bound_module, match_index, match_flags;
};

struct xv_kmod_match
{
  unsigned bus, flags;
  unsigned vendor, device, subsystem_vendor, subsystem_device;
  unsigned revision_min, revision_max;
  unsigned class_code, class_mask;
  char compatible[128];
};

struct xv_kmod_dependency
{
  char name[XV_KMOD_NAME];
};

struct xv_kmod_driver_v3
{
  struct xv_kmod_driver base;
  const struct xv_kmod_match *matches;
  unsigned match_count, dependency_count;
  const struct xv_kmod_dependency *dependencies;
  int (*attach)(const struct xv_kmod_host_v2 *, const struct xv_kmod_device_v3 *);
};

struct xv_kmod_status
{
  unsigned size, state, automatic, dependency_count;
  int last_error;
  char detail[128];
  struct xv_kmod_dependency dependencies[XV_KMOD_MAX_DEPENDENCIES];
};
