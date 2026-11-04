export module xv.userfs.catalog;
export namespace userfs::catalog
{
enum class Resource { None, Block, System, File };
struct Plugin
{
  char type[32]{}, alias[32]{}, executable[128]{}, image[64]{};
  char source[128]{}, options[256]{};
  Resource resource = Resource::None;
  unsigned mode = 0755, startup_ms = 20000;
  bool system = false, writable = false;
};
bool parse(const char *text, unsigned size, Plugin &);
bool installed(const char *type, Plugin &);
bool bundled(unsigned index, Plugin &);
int asset(const char *name);
bool mode(const char *text, unsigned &value);
}
