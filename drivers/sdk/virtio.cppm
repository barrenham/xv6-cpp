module xv.drivers:protocol;

namespace xv::drivers
{
//
// virtio device definitions.
// for both the mmio interface, and virtio descriptors.
// only tested with qemu.
//
// the virtio spec:
// https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
//

// virtio mmio control registers, mapped starting at 0x10001000.
// from qemu virtio_mmio.h
inline constexpr unsigned VIRTIO_MMIO_MAGIC_VALUE = 0x000; // 0x74726976
inline constexpr unsigned VIRTIO_MMIO_VERSION = 0x004;     // version; should be 2
inline constexpr unsigned VIRTIO_MMIO_DEVICE_ID =
    0x008; // device type; 1 is net, 2 is disk
inline constexpr unsigned VIRTIO_MMIO_VENDOR_ID = 0x00c; // 0x554d4551
inline constexpr unsigned VIRTIO_MMIO_DEVICE_FEATURES = 0x010;
inline constexpr unsigned VIRTIO_MMIO_DRIVER_FEATURES = 0x020;
inline constexpr unsigned VIRTIO_MMIO_DEVICE_FEATURES_SEL = 0x014;
inline constexpr unsigned VIRTIO_MMIO_DRIVER_FEATURES_SEL = 0x024;
inline constexpr unsigned VIRTIO_MMIO_QUEUE_SEL = 0x030; // select queue, write-only
inline constexpr unsigned VIRTIO_MMIO_QUEUE_NUM_MAX =
    0x034; // max size of current queue, read-only
inline constexpr unsigned VIRTIO_MMIO_QUEUE_NUM =
    0x038; // size of current queue, write-only
inline constexpr unsigned VIRTIO_MMIO_QUEUE_READY = 0x044;      // ready bit
inline constexpr unsigned VIRTIO_MMIO_QUEUE_NOTIFY = 0x050;     // write-only
inline constexpr unsigned VIRTIO_MMIO_INTERRUPT_STATUS = 0x060; // read-only
inline constexpr unsigned VIRTIO_MMIO_INTERRUPT_ACK = 0x064;    // write-only
inline constexpr unsigned VIRTIO_MMIO_STATUS = 0x070;           // read/write
inline constexpr unsigned VIRTIO_MMIO_QUEUE_DESC_LOW =
    0x080; // physical address for descriptor table, write-only
inline constexpr unsigned VIRTIO_MMIO_QUEUE_DESC_HIGH = 0x084;
inline constexpr unsigned VIRTIO_MMIO_DRIVER_DESC_LOW =
    0x090; // physical address for available ring, write-only
inline constexpr unsigned VIRTIO_MMIO_DRIVER_DESC_HIGH = 0x094;
inline constexpr unsigned VIRTIO_MMIO_DEVICE_DESC_LOW =
    0x0a0; // physical address for used ring, write-only
inline constexpr unsigned VIRTIO_MMIO_DEVICE_DESC_HIGH = 0x0a4;
inline constexpr unsigned VIRTIO_MMIO_CONFIG_GENERATION = 0x0fc;
inline constexpr unsigned VIRTIO_MMIO_CONFIG = 0x100;

// status register bits, from qemu virtio_config.h
inline constexpr unsigned VIRTIO_CONFIG_S_ACKNOWLEDGE = 1;
inline constexpr unsigned VIRTIO_CONFIG_S_DRIVER = 2;
inline constexpr unsigned VIRTIO_CONFIG_S_DRIVER_OK = 4;
inline constexpr unsigned VIRTIO_CONFIG_S_FEATURES_OK = 8;

// device feature bits
inline constexpr unsigned VIRTIO_BLK_F_RO = 5;    /* Disk is read-only */
inline constexpr unsigned VIRTIO_BLK_F_SCSI = 7;  /* Supports scsi command passthru */
inline constexpr unsigned VIRTIO_BLK_F_FLUSH = 9; /* Cache flush command supported */
inline constexpr unsigned VIRTIO_BLK_F_CONFIG_WCE =
    11;                                         /* Writeback mode available in config */
inline constexpr unsigned VIRTIO_BLK_F_MQ = 12; /* support more than one vq */
inline constexpr unsigned VIRTIO_F_ANY_LAYOUT = 27;
inline constexpr unsigned VIRTIO_RING_F_INDIRECT_DESC = 28;
inline constexpr unsigned VIRTIO_RING_F_EVENT_IDX = 29;

// this many virtio descriptors.
// must be a power of two.
inline constexpr int QueueSize = 8;

// a single descriptor, from the spec.
struct virtq_desc
{
  unsigned long addr;
  unsigned len;
  unsigned short flags;
  unsigned short next;
};

inline constexpr unsigned VRING_DESC_F_NEXT = 1;  // chained with another descriptor
inline constexpr unsigned VRING_DESC_F_WRITE = 2; // device writes (vs read)

// the (entire) avail ring, from the spec.
struct virtq_avail
{
  unsigned short flags;           // always zero
  unsigned short idx;             // driver will write ring[idx] next
  unsigned short ring[QueueSize]; // descriptor numbers of chain heads
  unsigned short unused;
};

// one entry in the "used" ring, with which the
// device tells the driver about completed requests.
struct virtq_used_elem
{
  unsigned id; // index of start of completed descriptor chain
  unsigned len;
};

struct virtq_used
{
  unsigned short flags; // always zero
  unsigned short idx;   // device increments when it adds a ring[] entry
  struct virtq_used_elem ring[QueueSize];
};

// these are specific to virtio block devices, e.g. disks,
// described in Section 5.2 of the spec.

inline constexpr unsigned VIRTIO_BLK_T_IN = 0;  // read the disk
inline constexpr unsigned VIRTIO_BLK_T_OUT = 1; // write the disk

// the format of the first descriptor in a disk request.
// to be followed by two more descriptors containing
// the block, and a one-byte status.
struct virtio_blk_req
{
  unsigned type; // VIRTIO_BLK_T_IN or ..._OUT
  unsigned reserved;
  unsigned long sector;
};

static_assert(sizeof(virtq_desc) == 16);
static_assert(sizeof(virtio_blk_req) == 16);
static_assert(sizeof(virtq_desc) * QueueSize <= 4096);
static_assert(sizeof(virtq_avail) <= 4096);
static_assert(sizeof(virtq_used) <= 4096);
static_assert((QueueSize & (QueueSize - 1)) == 0);
} // namespace xv::drivers
