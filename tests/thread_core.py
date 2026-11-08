#!/usr/bin/env python3
"""Exercise real kernel threads, shared process resources and group lifetime."""
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
with tempfile.TemporaryDirectory(prefix='xv-thread-core-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    guest = Guest(args, image, args.kernel.parent / f'thread-core-{args.cpus}cpu.log')
    try:
        login_root(guest.expect, guest.process)
        print(guest.command('/usr/tests/threadcore', 'threadcore: PASS', timeout=180))
        guest.command('echo thread-shell-survives', 'thread-shell-survives')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
print('Kernel thread core: shared resources, independent contexts, join and group termination PASS')
