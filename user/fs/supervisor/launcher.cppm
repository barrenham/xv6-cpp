export module xv.userfs.launcher;
export import xv.userfs.catalog;
export namespace userfs::launcher
{
int source(const catalog::Plugin &, const char *override_source = nullptr);
int probe(const catalog::Plugin &, int source);
int start(const catalog::Plugin &, const char *type, const char *path,
          int source, bool bundled, unsigned mode, int control = -1,
          const char *options = nullptr, bool (*progress)(void *) = nullptr,
          void *context = nullptr);
}
