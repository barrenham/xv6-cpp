#!/usr/bin/env python3
"""Verify named local sockets on ramfs, ext3 and xv6 using private disks."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image', 'mkfs'):
    parser.add_argument('--' + option, required=True, type=Path)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', required=True, type=int)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-unix-sockets-') as temporary:
    directory = Path(temporary)
    root = directory / 'root.img'
    shutil.copyfile(args.image, root)
    data = directory / 'legacy.img'
    manifest = directory / 'manifest.tsv'
    manifest.write_text('dir\t0755\t/volume\n')
    subprocess.run([str(args.mkfs), str(data), '--manifest', str(manifest),
                    '--filesystem', 'xv6'], check=True)
    guest = Guest(args, root, args.kernel.parent / f'unix-sockets-{args.cpus}cpu.log', data=data)
    try:
        login_root(guest.expect, guest.process)
        for path in ('/tmp/uds', '/root/uds'):
            print(guest.command('/usr/tests/unixsockettest ' + path,
                                'unixsockettest: PASS', timeout=180))
        guest.command('cd /tmp')
        print(guest.command('/usr/tests/unixsockettest uds', 'unixsockettest: PASS', timeout=180))
        guest.command('cd /')
        guest.command('mkdir /tmp/legacy')
        guest.command('mount -t xv6 -s 1 /tmp/legacy')
        print(guest.command('/usr/tests/unixsockettest /tmp/legacy/uds',
                            'unixsockettest: PASS', timeout=180))
        guest.command('umount /tmp/legacy')
        guest.command('ps', 'ipc-gc')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(root)], check=True, capture_output=True)
print('Named Unix sockets: independent exec, VFS paths, permissions and descriptor cycles PASS')
