"""Private in-memory ext3 editing through the kernel's shared disk engine.

The caller publishes bytes only after independent e2fsck validation. The bridge
never opens the live image or host paths found inside the guest filesystem.
"""
import ctypes as C
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


class Metadata(C.Structure):
    _fields_=[(name,C.c_uint32) for name in ('kind','uid','gid','mode','links','major','minor')]+[('size',C.c_uint64)]


def library():
    configured=os.environ.get('XV_EXT3_LIBRARY')
    root=Path(__file__).resolve().parents[1]
    candidates=[Path(configured)] if configured else [root/'build/dev/libext3_image.so',*sorted((root/'build').glob('*/libext3_image.so'))]
    path=next((path for path in candidates if path.is_file()),None)
    if path is None:raise ValueError('ext3 image library missing; build the host tools or set XV_EXT3_LIBRARY')
    lib=C.CDLL(str(path.resolve()))
    pointer=C.c_void_p;u32=C.c_uint32;u64=C.c_uint64
    signatures={
        'abi':([],C.c_uint),
        'open':([pointer,u64,C.POINTER(C.c_uint)],pointer),
        'close':([pointer],None),'error':([pointer],C.c_uint),
        'info':([pointer,u32,C.POINTER(Metadata)],C.c_int),
        'read':([pointer,u32,u64,pointer,C.c_uint],C.c_int),
        'next':([pointer,u32,C.POINTER(u64),pointer,C.POINTER(C.c_uint),C.POINTER(u32)],C.c_int),
        'attributes':([pointer,u32,u32,u32,C.c_uint],C.c_int),
        'create':([pointer,u32,C.c_char_p,C.c_uint,u32,u32,C.c_uint,C.POINTER(u32)],C.c_int),
        'link':([pointer,u32,C.c_char_p,u32],C.c_int),
        'unlink':([pointer,u32,C.c_char_p],C.c_int),
        'write':([pointer,u32,u64,pointer,u64],C.c_int),
        'device':([pointer,u32,u32,u32],C.c_int),
    }
    for name,(args,result) in signatures.items():
        function=getattr(lib,'xv_ext3_'+name);function.argtypes=args;function.restype=result
    if lib.xv_ext3_abi()!=1:raise ValueError('unsupported ext3 image library ABI')
    return lib


class Ext3Image:
    root=2
    filesystem='ext3'
    name_max=255

    @classmethod
    def from_bytes(cls,content):
        image=cls.__new__(cls)
        image.data=bytearray(content)
        image._buffer=(C.c_ubyte*len(image.data)).from_buffer(image.data)
        image._library=library()
        error=C.c_uint()
        image._handle=image._library.xv_ext3_open(image._buffer,len(image.data),C.byref(error))
        if not image._handle:raise ValueError(f'ext3 mount rejected image (error {error.value})')
        return image

    def close(self):
        if getattr(self,'_handle',None):
            self._library.xv_ext3_close(self._handle);self._handle=None

    def __del__(self):
        self.close()

    def _check(self,result):
        if result<0:raise ValueError(f'ext3 operation failed (error {self._library.xv_ext3_error(self._handle)})')
        return result

    def _info(self,number):
        if not 0<number<=0xffffffff:raise ValueError('invalid ext3 inode')
        info=Metadata();self._check(self._library.xv_ext3_info(self._handle,number,C.byref(info)));return info

    def kind(self,number):return self._info(number).kind
    def size(self,number):return self._info(number).size
    def attributes(self,number):
        info=self._info(number);return info.uid,info.gid,info.mode

    def set_attributes(self,number,attributes):
        uid,gid,mode=attributes
        if not 0<=uid<=0xffffffff or not 0<=gid<=0xffffffff or not 0<=mode<=0o7777:raise ValueError('invalid ext3 ownership or mode')
        self._check(self._library.xv_ext3_attributes(self._handle,number,uid,gid,mode))

    @staticmethod
    def _name(name):
        if not isinstance(name,bytes) or not 0<len(name)<=255 or b'\0' in name or b'/' in name:raise ValueError('invalid ext3 component name')
        return name

    def entries(self,number):
        offset=C.c_uint64();name=C.create_string_buffer(256);length=C.c_uint();child=C.c_uint32();entries={}
        while self._check(self._library.xv_ext3_next(self._handle,number,C.byref(offset),name,C.byref(length),C.byref(child))):
            key=name.raw[:length.value]
            if key in entries:raise ValueError('duplicate ext3 directory entry')
            entries[key]=child.value
        return entries

    def create(self,parent,name,kind,attributes=(0,0,0o644)):
        uid,gid,mode=attributes
        if not 0<=uid<=0xffffffff or not 0<=gid<=0xffffffff or not 0<=mode<=0o7777:raise ValueError('invalid ext3 ownership or mode')
        child=C.c_uint32();self._check(self._library.xv_ext3_create(self._handle,parent,self._name(name),kind,uid,gid,mode,C.byref(child)));return child.value

    def link(self,parent,name,number):self._check(self._library.xv_ext3_link(self._handle,parent,self._name(name),number))
    def unlink_file(self,parent,name):
        number=self.entries(parent).get(name)
        if number is None or self.kind(number) not in (2,5):raise ValueError('system program target is not a regular file or symlink')
        self._check(self._library.xv_ext3_unlink(self._handle,parent,self._name(name)))

    def read(self,number):
        size=self.size(number);result=bytearray();block=C.create_string_buffer(65536)
        while len(result)<size:
            count=min(size-len(result),len(block))
            copied=self._check(self._library.xv_ext3_read(self._handle,number,len(result),block,count))
            if copied!=count:raise ValueError('short ext3 inode read')
            result.extend(block.raw[:copied])
        return bytes(result)

    def append(self,number,content):
        content=bytes(content)
        self._check(self._library.xv_ext3_write(self._handle,number,self.size(number),content,len(content)))

    def write_at(self,number,offset,content):
        content=bytes(content)
        if offset<0 or offset+len(content)>self.size(number):raise ValueError('write outside existing inode size')
        self._check(self._library.xv_ext3_write(self._handle,number,offset,content,len(content)))

    def device_numbers(self,number):
        info=self._info(number);return info.major,info.minor

    def set_device_numbers(self,number,values):
        major,minor=values
        if not 0<=major<=4095 or not 0<=minor<=1048575:raise ValueError('invalid ext3 device number')
        self._check(self._library.xv_ext3_device(self._handle,number,major,minor))

    def import_volume(self,source):
        from create_rootfs import import_tree
        import_tree(self,source,rewrite_legacy_home=True)

    def validate_allocations(self):
        # e2fsck is independent of the engine: it checks bitmaps, counts,
        # ownership, directory structure and inode references before publishing.
        checker=shutil.which('e2fsck') or '/usr/sbin/e2fsck'
        with tempfile.TemporaryDirectory(prefix='ext3-check-') as temporary:
            path=Path(temporary)/'image'
            with path.open('xb') as stream:
                os.chmod(path,0o600);stream.write(self.data)
            result=subprocess.run([checker,'-fn',str(path)],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            if result.returncode:raise ValueError(f'ext3 allocation validation failed (e2fsck status {result.returncode})')
