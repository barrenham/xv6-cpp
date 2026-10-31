#!/usr/bin/env python3
"""Read standard ext3 images created independently by e2fsprogs."""
import argparse
from pathlib import Path
import shutil
import re
import subprocess
import tempfile


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--contract',required=True)
    parser.add_argument('--fs-contract')
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='ext3-volume-') as temporary:
        root=Path(temporary)
        payload=root/'payload'
        size=2*1024*1024+17
        payload.write_bytes((bytes((i*37+11)&255 for i in range(256))*(size//256+1))[:size])
        for block in (1024,2048,4096):
            image=root/f'ext3-{block}.img'
            with image.open('wb') as stream:stream.truncate(32*1024*1024)
            subprocess.run([shutil.which('mke2fs') or '/usr/sbin/mke2fs','-q','-F','-t','ext3','-b',str(block),str(image)],check=True)
            commands=root/'debugfs.commands'
            commands.write_text(f'write {payload} /payload\nsymlink /shortlink payload\nmkdir /dev\nmkdir /proc\nmkdir /tmp\nmkdir /data\n')
            subprocess.run([shutil.which('debugfs') or '/usr/sbin/debugfs','-w','-f',str(commands),str(image)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
            subprocess.run([args.contract,str(image)],check=True)
            subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fn',str(image)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
            if args.fs_contract:
                subprocess.run([args.fs_contract,'--ext3',str(image)],check=True)
                subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fn',str(image)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
                result=subprocess.run([shutil.which('debugfs') or '/usr/sbin/debugfs','-R','cat /module-kept',str(image)],check=True,capture_output=True)
                assert result.stdout==b'VFS and JBD', result.stdout
                metadata=subprocess.run([shutil.which('debugfs') or '/usr/sbin/debugfs','-R','stat /module-kept',str(image)],check=True,capture_output=True).stdout
                timestamps=re.findall(rb'^\s*(?:ctime|mtime):\s+0x([0-9a-fA-F]+)',metadata,re.MULTILINE)
                assert len(timestamps)==2 and all(int(value,16)==1700000000 for value in timestamps), metadata
                secondary=Path(str(image)+'.data')
                subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fn',str(secondary)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
                result=subprocess.run([shutil.which('debugfs') or '/usr/sbin/debugfs','-R','cat /data-kept',str(secondary)],check=True,capture_output=True)
                assert result.stdout==b'secondary ext3', result.stdout
            orphan=root/'orphan.img';shutil.copyfile(image,orphan)
            expected=subprocess.run([args.contract,str(orphan),'--orphan-prepare','0'],check=True,capture_output=True).stdout.split()
            for cut in (0,1,2,3,4,5,8,13,21,34,55,89,144,233):
                crashed=root/'orphan-cut.img';shutil.copyfile(orphan,crashed)
                subprocess.run([args.contract,str(crashed),'--orphan-cut',str(cut)],check=True,capture_output=True)
                own=root/'orphan-own.img';shutil.copyfile(crashed,own)
                actual=subprocess.run([args.contract,str(own),'--orphan-check','0'],check=True,capture_output=True).stdout.split()
                assert actual==expected,(block,cut,actual,expected)
                subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fn',str(own)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
                checked=subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fy',str(crashed)],capture_output=True,text=True)
                assert checked.returncode in (0,1),(block,cut,checked.stdout,checked.stderr)
                actual=subprocess.run([args.contract,str(crashed),'--orphan-check','0'],check=True,capture_output=True).stdout.split()
                assert actual==expected,(block,cut,actual,expected)
            print(f'ext3 {block}-byte blocks: 14 orphan recovery cut points checked by native and Linux recovery',flush=True)
            print(f'ext3 {block}-byte blocks: e2fsprogs image and checker passed',flush=True)
            for cut in range(100):
                crash=root/'crash.img'
                shutil.copyfile(image,crash)
                result=subprocess.run([args.contract,str(crash),'--crash',str(cut)],check=True,capture_output=True,text=True)
                complete,committed,events=map(int,result.stdout.split())
                own=root/'own-recovery.img'
                shutil.copyfile(crash,own)
                subprocess.run([args.contract,str(own),'--recover',str(committed)],check=True)
                subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fn',str(own)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
                # Independently replay our on-disk JBD records with e2fsprogs.
                checked=subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck','-fy',str(crash)],capture_output=True,text=True)
                if checked.returncode not in (0,1):
                    raise AssertionError(f'Linux recovery failed at block={block}, cut={cut}: {checked.stdout}{checked.stderr}')
                result=subprocess.run([shutil.which('debugfs') or '/usr/sbin/debugfs','-R','cat /atomic-crash',str(crash)],check=True,capture_output=True)
                assert result.stdout in (b'',b'durable'), (block,cut,result.stdout)
                if committed:assert result.stdout==b'durable', (block,cut,'lost committed transaction')
                if complete:
                    print(f'ext3 {block}-byte blocks: all {cut+1} commit cut points recovered by native JBD and Linux',flush=True)
                    break
            else:raise AssertionError('commit did not finish within 100 I/O events')


if __name__=='__main__':main()
