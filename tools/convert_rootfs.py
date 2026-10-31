#!/usr/bin/env python3
"""Convert an offline rootfs with a full backup and atomic publication.

Names, contents, hard links, symbolic links, ownership, modes and device numbers
are compared before publication. Existing account records are copied verbatim.
"""
import argparse
from datetime import datetime,timezone
import fcntl
import hashlib
import os
from pathlib import Path
import stat
import struct
import subprocess
import tempfile
import uuid

from create_rootfs import Image,DIRECTORY,DEVICE,BLOCK_DEVICE,import_tree
from update_rootfs import fsync_directory,same_inode


def snapshot(image):
    records={};links={};directories=set()
    def visit(number,path):
        kind=image.kind(number)
        if kind==DIRECTORY:
            if number in directories:raise ValueError('directory cycle in source image')
            directories.add(number)
            records[path]=(kind,image.attributes(number))
            for name,child in image.entries(number).items():
                if name not in (b'.',b'..'):visit(child,path+b'/'+name)
        else:
            if kind not in (2,DEVICE,5,BLOCK_DEVICE):raise ValueError('unsupported source inode type')
            content=image.read(number)
            records[path]=(kind,image.attributes(number),len(content),hashlib.sha256(content).digest(),image.device_numbers(number) if kind in (DEVICE,BLOCK_DEVICE) else None)
            links.setdefault(number,[]).append(path)
    visit(image.root,b'')
    groups={tuple(sorted(paths)) for paths in links.values() if len(paths)>1}
    return records,groups


def verify(source,target):
    before,old_links=snapshot(source);after,new_links=snapshot(target)
    extra=set(after)-set(before)
    # mke2fs creates this empty recovery directory. It carries no source data.
    if extra=={b'/lost+found'}:
        child=target.entries(target.root)[b'lost+found']
        if target.kind(child)!=DIRECTORY or set(target.entries(child))!={b'.',b'..'}:raise ValueError('unexpected target recovery directory')
        del after[b'/lost+found']
    if before!=after or old_links!=new_links:raise ValueError('converted image differs from source contents, links or metadata')
    return len(before)


def convert(path,mkfs,filesystem='ext3',dry_run=False):
    path=path.resolve(strict=True)
    with path.open('r+b') as original:
        if not stat.S_ISREG(os.fstat(original.fileno()).st_mode):raise ValueError('rootfs is not a regular image')
        fcntl.lockf(original,fcntl.LOCK_EX|fcntl.LOCK_NB)
        if not same_inode(path,original.fileno()):raise ValueError('image path changed while acquiring its lock; retry')
        content=original.read();source=Image.from_bytes(content);source.validate_allocations()
        if source.filesystem==filesystem:
            print('rootfs already uses '+filesystem);return
        with tempfile.TemporaryDirectory(prefix='.rootfs-convert-',dir=path.parent) as temporary:
            temporary=Path(temporary);base=temporary/'base.img';manifest=temporary/'empty.tsv';manifest.write_text('')
            subprocess.run([str(mkfs.resolve(strict=True)),str(base),'--manifest',str(manifest),'--filesystem',filesystem],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            target=Image(base)
            target.set_attributes(target.root,source.attributes(source.root))
            import_tree(target,source)
            target.validate_allocations();count=verify(source,target)
            if dry_run:
                print(f'dry run: {source.filesystem} -> {filesystem}; {count} paths verified; image unchanged');return
            candidate=temporary/'converted.img'
            with candidate.open('x+b') as output:
                os.chmod(candidate,0o600);fcntl.lockf(output,fcntl.LOCK_EX|fcntl.LOCK_NB)
                output.write(target.data);output.flush();os.fsync(output.fileno())
                # Check serialized bytes as well as the in-memory target view.
                output.seek(0);restored=Image.from_bytes(output.read());restored.validate_allocations();verify(source,restored)
                stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
                backup=path.with_name(path.name+'.before-convert-'+stamp+'-'+uuid.uuid4().hex[:8]+'.img')
                before=temporary/'before.img'
                with before.open('xb') as saved:
                    os.chmod(before,0o600);saved.write(content);saved.flush();os.fsync(saved.fileno())
                if not same_inode(path,original.fileno()):raise ValueError('image path changed during conversion; retry')
                os.link(before,backup);fsync_directory(path.parent)
                os.replace(candidate,path);fsync_directory(path.parent)
        print(f'rootfs converted: {source.filesystem} -> {filesystem}; {count} paths verified')
        print('backup:',backup)


def main():
    root=Path(__file__).resolve().parents[1]
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image',required=True,type=Path)
    parser.add_argument('--mkfs',type=Path,default=root/'build/dev/mkfs')
    parser.add_argument('--filesystem',choices=('ext3','xv6'),default='ext3')
    parser.add_argument('--dry-run',action='store_true')
    args=parser.parse_args();convert(args.image,args.mkfs,args.filesystem,args.dry_run)


if __name__=='__main__':
    try:main()
    except BlockingIOError:raise SystemExit('rootfs-convert: image is in use; exit QEMU before converting')
    except (OSError,ValueError,struct.error,subprocess.CalledProcessError) as error:raise SystemExit('rootfs-convert: '+str(error))
