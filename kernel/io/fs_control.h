#pragma once

// Carrier ABI only. Namespaces, command numbers and payloads belong to the ELF.
namespace xv::fscontrol
{
inline constexpr unsigned Ioctl = 0x58560007U, Version = 1, DataSize = 512;
inline constexpr unsigned Readable = 1, Writable = 2, ReadOnly = 4;
struct Request
{
  unsigned size = sizeof(Request), version = Version;
  unsigned driver_version = 1, command = 0;
  unsigned input_size = 0, capacity = DataSize, output_size = 0;
  char interface_name[32]{};
  unsigned char data[DataSize]{};
};
}
