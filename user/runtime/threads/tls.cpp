#include "user/runtime/user.h"
#include "internal.h"

extern "C"
{
  extern const char __xv_tls_image[], __xv_tls_file_bytes[], __xv_tls_bytes[],
      __xv_tls_align[];
  extern char __xv_main_thread[];
}

namespace xv_threads
{
namespace
{
constexpr unsigned long HeaderBytes = 128;
static_assert(sizeof(Control) + sizeof(Control *) <= HeaderBytes);

Control *
initialize(char *base, unsigned long stack, unsigned long bytes, unsigned long header,
           bool image_owned) {
  auto *control = reinterpret_cast<Control *>(base + (stack ? Page + stack : 0));
  auto *tp = reinterpret_cast<char *>(control) + header;
  *control = {};
  control->mapping = image_owned ? nullptr : base;
  control->mapping_bytes = image_owned ? 0 : bytes;
  control->stack_top = reinterpret_cast<unsigned long>(control);
  control->thread_pointer = reinterpret_cast<unsigned long>(tp);
  *reinterpret_cast<Control **>(tp - sizeof(Control *)) = control;
  auto file_bytes = reinterpret_cast<unsigned long>(__xv_tls_file_bytes);
  if (file_bytes)
  {
    auto alignment = reinterpret_cast<unsigned long>(__xv_tls_align);
    auto image_offset =
        reinterpret_cast<unsigned long>(__xv_tls_image) & (alignment - 1);
    memmove(tp + image_offset, __xv_tls_image, file_bytes);
  }
  return control;
}
} // namespace

Control *
initialize_main() {
  auto tls_bytes = reinterpret_cast<unsigned long>(__xv_tls_bytes);
  auto file_bytes = reinterpret_cast<unsigned long>(__xv_tls_file_bytes);
  auto alignment = reinterpret_cast<unsigned long>(__xv_tls_align);
  if (tls_bytes > 256 * 1024 || file_bytes > tls_bytes || alignment < 16 ||
      alignment > Page || (alignment & (alignment - 1)))
  {
    return nullptr;
  }
  // RV64 local-exec relocations use the aligned TLS block origin as TP;
  // initialized template bytes can begin at a nonzero offset from that origin.
  auto header = (HeaderBytes + alignment - 1) & ~(alignment - 1);
  return initialize(__xv_main_thread, 0, 0, header, true);
}

Control *
allocate(unsigned long stack) {
  auto tls_bytes = reinterpret_cast<unsigned long>(__xv_tls_bytes);
  auto file_bytes = reinterpret_cast<unsigned long>(__xv_tls_file_bytes);
  auto alignment = reinterpret_cast<unsigned long>(__xv_tls_align);
  if (stack > 512 * 1024 || tls_bytes > 256 * 1024 || file_bytes > tls_bytes ||
      alignment < 16 || alignment > Page || (alignment & (alignment - 1)))
  {
    return nullptr;
  }
  stack = (stack + Page - 1) & ~(Page - 1);
  // Keep the control header and ordinary TLS in one page. The back-pointer
  // immediately before TP is outside Control. The guarded stack ends at Control.
  // RV64 local-exec relocations use the aligned TLS block origin as TP;
  // initialized template bytes can begin at a nonzero offset from that origin.
  auto header = (HeaderBytes + alignment - 1) & ~(alignment - 1);
  auto image_offset = reinterpret_cast<unsigned long>(__xv_tls_image) & (alignment - 1);
  auto bytes = (stack ? Page + stack : 0) +
               ((header + image_offset + tls_bytes + Page - 1) & ~(Page - 1));
  auto *base = static_cast<char *>(
      mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (base == MAP_FAILED)
  {
    return nullptr;
  }
  if (stack && mprotect(base, Page, PROT_NONE))
  {
    munmap(base, bytes);
    return nullptr;
  }
  return initialize(base, stack, bytes, header, false);
}

void
release(Control *control) {
  // The initial thread's storage is part of the process image and survives
  // joining that thread until the last remaining thread exits.
  if (control->mapping)
  {
    munmap(control->mapping, control->mapping_bytes);
  }
}
} // namespace xv_threads
