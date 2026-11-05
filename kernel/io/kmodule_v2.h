#pragma once
#include "kmodule.h"
#include <stdbool.h>

#define XV_KMOD_ABI_V2 2U
#define XV_KMOD_DEVICE_PCI_READY 1U
#define XV_KMOD_BUS_PLATFORM 2U
#define XV_KMOD_BUS_SOFTWARE 3U
#define XV_KMOD_BUS_PCI 4U

// Generic platform devices use the first DT compatible string as a stable ID.
// Device-specific bind code must still validate its hardware before programming it.
#ifdef __cplusplus
constexpr
#else
static inline
#endif
unsigned xv_kmod_compatible_id(const char *name)
{
  unsigned value = 2166136261U;
  while (*name) { value ^= (unsigned char)*name++; value *= 16777619U; }
  return value ? value : 1U;
}

struct xv_kmod_device_v2
{
  struct xv_kmod_device base;
  unsigned slot;
  unsigned flags;
  void *binding;
};

struct xv_kmod_buffer
{
  unsigned long address;
  bool user;
  bool nonblocking;
};

struct xv_kmod_character
{
  void *context;
  void *(*open)(void *, bool);
  void (*close)(void *);
  int (*read)(void *, struct xv_kmod_buffer, int);
  int (*write)(void *, struct xv_kmod_buffer, int);
  int (*ioctl)(void *, unsigned, struct xv_kmod_buffer);
  unsigned (*poll)(void *, void *);
};

struct xv_kmod_serial
{
  void *context;
  int (*transmit)(void *, const char *, int, bool);
  void (*put)(void *, int);
  void (*drain)(void *);
  bool (*writable)(void *, void *);
};

struct xv_kmod_ethernet
{
  void *context;
  bool (*online)(void *);
  bool (*carrier)(void *);
  void (*mac)(void *, unsigned char *);
  bool (*transmit)(void *, void *, unsigned);
  bool (*promiscuous)(void *, bool);
};

// Each ELF has one immutable host table and owns its pages for its resident
// lifetime. Binding tokens identify hardware instances; C++ objects remain on
// their own side of this interface. Probe failures must undo their allocations.
struct xv_kmod_host_v2
{
  unsigned size;
  unsigned abi;
  void *owner;
  void *(*page)(void *);
  void (*free_page)(void *, void *, bool);
  void (*push_off)();
  void (*pop_off)();
  void *(*cpu)();
  int (*pid)();
  void (*prepare_sleep)(void *);
  void (*prepare_interruptible_sleep)(void *);
  void (*sleep)();
  void (*wake)(void *);
  void (*wake_one)(void *);
  bool (*killed)();
  void (*panic)(const char *);
  bool (*panicking)();
  bool (*panicked)();
  unsigned (*uid)();
  unsigned (*gid)();
  unsigned long (*milliseconds)();
  int (*copy_in)(void *, bool, unsigned long, unsigned);
  int (*copy_out)(bool, unsigned long, void *, unsigned);
  int (*wait_completion)(void *, volatile unsigned short *, unsigned short,
                         unsigned long);
  void (*poll_watch)(void *, void *);
  unsigned (*poll_events)(void *);
  unsigned long (*map_pages)(void *const *, unsigned);
  bool (*mapped_pages)(void *const *, unsigned);
  bool (*unmap_pages)();
  int (*character)(void *, const char *, unsigned, unsigned, unsigned,
                   const struct xv_kmod_character *);
  int (*serial)(void *, const struct xv_kmod_serial *);
  int (*ethernet)(void *, const struct xv_kmod_ethernet *);
  int (*interrupt)(void *, void *, void (*)(void *));
  void (*serial_input)(void *, int);
  void (*ethernet_input)(void *, void *, unsigned);
};

struct xv_kmod_driver_v2
{
  // base.bind is null; base.size covers this entire record. ABI 1 ELFs retain
  // their original descriptor and are still supported by the loader.
  struct xv_kmod_driver base;
  int (*attach)(const struct xv_kmod_host_v2 *, const struct xv_kmod_device_v2 *);
};
