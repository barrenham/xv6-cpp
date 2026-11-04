#pragma once

// RV64 LP64 C ABI. Modules are trusted supervisor code, not user executables.
// No C++ objects or kernel-private symbols cross this interface.
#define XV_KMOD_ABI 1U
#define XV_KMOD_MAX_FILE (256U * 1024)
#define XV_KMOD_NAME 32
#define XV_KMOD_BUS_VIRTIO_MMIO 1U
#define XV_KMOD_LOAD 0
#define XV_KMOD_INFO 1
#define XV_KMOD_DEVICE 2
#define XV_KMOD_BIND 3

struct xv_kmod_device
{
  unsigned size, bus, id, vendor, irq, bound;
  unsigned long mmio, mmio_size;
  char name[XV_KMOD_NAME];
};

struct xv_kmod_info
{
  unsigned size, abi, id, bound_devices;
  unsigned long base, bytes, transfers, interrupts;
  char name[XV_KMOD_NAME];
};

struct xv_kmod_host
{
  unsigned size, abi;
  void *owner;
  // Allocated pages and locks belong to this binding. They remain resident on
  // success and are reclaimed after a failed, quiesced probe.
  void *(*page)(void *owner);
  void *(*lock)(void *owner);
  void (*acquire)(void *lock);
  void (*release)(void *lock);
  // Atomically subscribe, drop the held spinlock, sleep, and reacquire it.
  void (*wait)(void *lock, void *channel);
  void (*wake)(void *channel);
};

struct xv_kmod_block
{
  unsigned size, flags; // reserved; ABI 1 requires a writable block device
  void *context;
  unsigned long sectors;
  int (*transfer)(void *context, unsigned write, unsigned long sector, void *dma_buffer,
                  unsigned bytes);
  int (*flush)(void *context);
  void (*interrupt)(void *context);
};

struct xv_kmod_driver
{
  unsigned size, abi, bus, device_id;
  char name[XV_KMOD_NAME];
  // Failure must reset hardware and stop DMA before returning. The host then
  // reclaims binding allocations. Success publishes one resident block device.
  int (*bind)(const struct xv_kmod_host *, const struct xv_kmod_device *,
              struct xv_kmod_block *);
};

typedef const struct xv_kmod_driver *(*xv_kmod_entry)(unsigned host_abi);
