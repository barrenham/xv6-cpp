module xv.drivers;

namespace xv::drivers
{
bool retain_named_pty(unsigned) { return false; }
void release_named_pty(unsigned) {}
CharacterDevice *open_named_pty(unsigned) { return nullptr; }
unsigned named_pty_ids(unsigned *, unsigned) { return 0; }
}
