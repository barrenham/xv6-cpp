import xv.fs;

extern "C" xv::fs::File *filedup(xv::fs::File *file)
{
  return xv::fs::files().retain(file);
}

extern "C" void fileclose(xv::fs::File *file)
{
  xv::fs::files().release(file);
}

extern "C" xv::fs::Inode *idup(xv::fs::Inode *node)
{
  return xv::fs::filesystem().retain(node);
}

extern "C" void iput(xv::fs::Inode *node)
{
  xv::fs::filesystem().release(node);
}
