#!/usr/bin/env python3
"""Exercise both image formats, private-state conversion and offline upgrades."""
import argparse
import fcntl
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from unittest import mock

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from create_rootfs import Image,DIRECTORY,FILE,DEVICE,SYMLINK
import convert_rootfs


def run(script,*arguments,success=True):
    result=subprocess.run([sys.executable,str(ROOT/'tools'/script),*map(str,arguments)],capture_output=True)
    assert (result.returncode==0)==success,(script,result.stdout,result.stderr)
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mkfs',required=True,type=Path)
    parser.add_argument('--legacy-seed',required=True,type=Path)
    parser.add_argument('--ext3-seed',required=True,type=Path)
    parser.add_argument('--manifest',required=True,type=Path)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='ext3 image tools ') as temporary:
        if Image(args.ext3_seed).filesystem!='ext3':
            ext3_seed=Path(temporary)/'ext3-seed.img'
            subprocess.run([str(args.mkfs),str(ext3_seed),'--manifest',str(args.manifest),'--filesystem','ext3'],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            args.ext3_seed=ext3_seed
        root=Path(temporary);image_path=root/'long names.img';manifest=root/'manifest.tsv';payload=root/'payload'
        payload.write_bytes(bytes(range(256))*8193)
        long_name='n'*255
        manifest.write_text(f'dir\t0700\t/long\nfile\t0640\t/long/{long_name}\t{payload}\n')
        subprocess.run([str(args.mkfs),str(image_path),'--manifest',str(manifest)],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        image=Image(image_path);assert image.filesystem=='ext3'
        folder=image.entries(image.root)[b'long'];node=image.entries(folder)[long_name.encode()]
        assert image.read(node)==payload.read_bytes() and image.attributes(node)==(0,0,0o640)
        image.validate_allocations();original=image_path.read_bytes()
        managed_root=root/'managed-install';managed_root.mkdir()
        managed=managed_root/'managed.tsv';managed.write_text(f'dir\t0755\t/bin\nfile\t0640\t/bin/{long_name}\t{payload}\n')
        managed_seed=managed_root/'long seed.img'
        subprocess.run([str(args.mkfs),str(managed_seed),'--manifest',str(managed)],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        installed=managed_root/'long installed.img';shutil.copyfile(managed_seed,installed)
        changed=Image(installed);folder=changed.entries(changed.root)[b'bin'];entry=changed.entries(folder)[long_name.encode()]
        changed.set_attributes(entry,(0,0,0o600));installed.write_bytes(changed.data)
        run('update_rootfs.py','--seed',managed_seed,'--image',installed,'--manifest',managed)
        updated=Image(installed);folder=updated.entries(updated.root)[b'bin'];entry=updated.entries(folder)[long_name.encode()]
        assert updated.attributes(entry)==(0,0,0o640) and updated.read(entry)==payload.read_bytes()
        assert image_path.read_bytes()==original
        # Existing output remains byte-for-byte intact if formatting/population fails.
        for text in ('dir\t0755\t/missing/parent\n','dir\t0755\t/dup\ndir\t0755\t/dup\n',f'file\t0644\t/file\t{root/"absent"}\n'):
            manifest.write_text(text)
            result=subprocess.run([str(args.mkfs),str(image_path),'--manifest',str(manifest)],capture_output=True)
            assert result.returncode and image_path.read_bytes()==original
            assert not list(root.glob('*.tmp.*'))
        # An ext3 file larger than the retained xv6 limit cannot be silently
        # truncated by a reverse conversion; the source remains untouched.
        run('convert_rootfs.py','--image',image_path,'--mkfs',args.mkfs,'--filesystem','xv6',success=False)
        assert image_path.read_bytes()==original and not list(root.glob('*.before-convert-*.img'))

        target=root/'private-rootfs.img';shutil.copyfile(args.legacy_seed,target)
        private=Image(target);top=private.entries(private.root);etc=top[b'etc'];home=top[b'home']
        alice=private.create(home,b'alice',DIRECTORY,(1000,1001,0o750))
        content=b'PRIVATE\0USER DATA\n'*900
        note=private.create(alice,b'note',FILE,(1000,1001,0o6750));private.append(note,content)
        private.link(alice,b'alias',note)
        symbolic=private.create(alice,b'link',SYMLINK,(1000,1001,0o777));private.append(symbolic,b'note')
        device=private.create(top[b'dev'],b'testchar',DEVICE,(0,0,0o600));private.set_device_numbers(device,(5,0))
        # Conversion must not apply the separate legacy /data import rewrite.
        for name,data,mode in ((b'passwd',b'root:!:0:0:root:/data/root:/bin/sh\n',0o644),(b'shadow',b'OPAQUE PRIVATE CREDENTIAL\0BYTES\n',0o600)):
            number=private.create(etc,name,FILE,(0,0,mode));private.append(number,data)
        private.validate_allocations();target.write_bytes(private.data);before=target.read_bytes()
        run('convert_rootfs.py','--image',target,'--mkfs',args.mkfs,'--dry-run')
        assert target.read_bytes()==before and not list(root.glob('*.before-convert-*.img'))
        with target.open('r+b') as lock:
            fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
            run('convert_rootfs.py','--image',target,'--mkfs',args.mkfs,success=False)
        assert target.read_bytes()==before
        # A path race after building/verifying the candidate must not publish.
        with mock.patch.object(convert_rootfs,'same_inode',side_effect=[True,False]):
            try:convert_rootfs.convert(target,args.mkfs)
            except ValueError:pass
            else:raise AssertionError('image path race was ignored')
        assert target.read_bytes()==before and not list(root.glob('*.before-convert-*.img'))
        run('convert_rootfs.py','--image',target,'--mkfs',args.mkfs)
        converted=Image(target);assert converted.filesystem=='ext3'
        convert_rootfs.verify(private,converted)
        backups=list(root.glob('*.before-convert-*.img'));assert len(backups)==1 and backups[0].read_bytes()==before and backups[0].stat().st_mode&0o777==0o600
        assert target.stat().st_mode&0o777==0o600
        migrated=target.read_bytes();run('convert_rootfs.py','--image',target,'--mkfs',args.mkfs)
        assert target.read_bytes()==migrated and len(list(root.glob('*.before-convert-*.img')))==1

        top=converted.entries(converted.root);alice=converted.entries(top[b'home'])[b'alice'];binary=top[b'bin']
        converted.unlink_file(binary,b'echo')
        old=converted.create(binary,b'echo',FILE,(0,0,0o755));converted.append(old,b'OBSOLETE SYSTEM PROGRAM\n')
        converted.link(alice,b'old-program',old)
        target.write_bytes(converted.data);before_update=target.read_bytes()
        run('update_rootfs.py','--seed',args.ext3_seed,'--image',target,'--manifest',args.manifest,'--dry-run')
        assert target.read_bytes()==before_update and not list(root.glob('*.before-update-*.img'))
        run('update_rootfs.py','--seed',args.ext3_seed,'--image',target,'--manifest',args.manifest)
        updated=Image(target);fresh=Image(args.ext3_seed);new_binary=updated.entries(updated.root)[b'bin'];fresh_binary=fresh.entries(fresh.root)[b'bin']
        assert updated.read(updated.entries(new_binary)[b'echo'])==fresh.read(fresh.entries(fresh_binary)[b'echo'])
        updated_alice=updated.entries(updated.entries(updated.root)[b'home'])[b'alice']
        names=updated.entries(updated_alice);assert names[b'note']==names[b'alias'] and updated.read(names[b'note'])==content
        assert updated.read(names[b'old-program'])==b'OBSOLETE SYSTEM PROGRAM\n'
        assert updated.attributes(names[b'note'])==(1000,1001,0o6750)
        updated_etc=updated.entries(updated.root)[b'etc']
        assert updated.read(updated.entries(updated_etc)[b'shadow'])==b'OPAQUE PRIVATE CREDENTIAL\0BYTES\n'
        backups=list(root.glob('*.before-update-*.img'));assert len(backups)==1 and backups[0].read_bytes()==before_update and backups[0].stat().st_mode&0o777==0o600
        updated.validate_allocations();after=target.read_bytes()
        run('update_rootfs.py','--seed',args.ext3_seed,'--image',target,'--manifest',args.manifest)
        assert target.read_bytes()==after and len(list(root.glob('*.before-update-*.img')))==1
        clean=root/'fresh.img';run('create_rootfs.py','--seed',args.ext3_seed,'--output',clean,'--fresh')
        assert Image(clean).filesystem=='ext3'
        run('create_rootfs.py','--seed',root/'missing-seed','--output',clean)
    print('ext3 image tools: default/legacy formats, long names, rollback, locked conversion, exact backups, private state and program updates passed')


if __name__=='__main__':main()
