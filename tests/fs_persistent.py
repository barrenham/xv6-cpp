#!/usr/bin/env python3
"""Exercise shared backend storage and persistent parallel read/write workers."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image'):
    parser.add_argument('--' + option, type=Path, required=True)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', type=int, required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-fs-persistent-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    guest = Guest(args, image, args.kernel.parent / f'fs-persistent-{args.cpus}cpu.log')
    try:
        login_root(guest.expect, guest.process)
        print(guest.command('/usr/tests/sharedheap', 'sharedheap: PASS', timeout=60))
        print(guest.command('/usr/tests/fspooltest', 'fspooltest: PASS', timeout=90))
        guest.command('mkdir /tmp/persistent')
        guest.command('mount -t ramfs -o workers=2 /tmp/persistent')
        print(guest.command('/usr/tests/demandcachetest /tmp/persistent/data',
                            'demandcachetest: PASS', timeout=120))
        print(guest.command('/usr/tests/filemaptest /tmp/persistent/mapped',
                            'filemaptest: PASS', timeout=180))
        print(guest.command('/usr/tests/fscachetest /tmp/persistent/cache',
                            'fscachetest: PASS', timeout=120))
        guest.command('umount /tmp/persistent')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
print('Persistent FS: shared storage, read/write overlap, stable workers, metadata barriers and ramfs coherence PASS')
