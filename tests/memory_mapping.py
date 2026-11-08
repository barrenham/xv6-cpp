#!/usr/bin/env python3
"""Exercise anonymous mappings and memory protection using a disposable disk."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image', 'mkfs'):
    parser.add_argument('--' + option, type=Path, required=True)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', type=int, required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-memory-mapping-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    data = Path(temporary) / 'legacy.img'
    manifest = Path(temporary) / 'manifest.tsv'
    manifest.write_text('dir\t0755\t/volume\n')
    subprocess.run([str(args.mkfs), str(data), '--manifest', str(manifest),
                    '--filesystem', 'xv6'], check=True)
    guest = Guest(args, image, args.kernel.parent / f'memory-mapping-{args.cpus}cpu.log',
                  allow_user_faults=True, data=data)
    try:
        login_root(guest.expect, guest.process)
        output = guest.command('/usr/tests/mappingtest', 'mappingtest: PASS', timeout=120)
        print(output)
        assert output.count('usertrap(): unexpected') == 2, output
        output = guest.command('/usr/tests/cachereclaim', 'cachereclaim: PASS', timeout=180)
        print(output)
        assert 'usertrap(): unexpected' not in output, output
        for path in ('/tmp/filemaptest', '/root/filemaptest'):
            output = guest.command('/usr/tests/filemaptest ' + path,
                                   'filemaptest: PASS', timeout=180)
            print(output)
            assert output.count('usertrap(): unexpected') == 0, output
        guest.command('mkdir /tmp/legacy')
        guest.command('mount -t xv6 -s 1 /tmp/legacy')
        output = guest.command('/usr/tests/filemaptest /tmp/legacy/filemaptest',
                               'filemaptest: PASS', timeout=180)
        print(output)
        assert output.count('usertrap(): unexpected') == 0, output
        guest.command('umount /tmp/legacy')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
print('Memory mapping: anonymous, ramfs/ext3/xv6 files, protections, fork, SIGBUS, SMP and cleanup PASS')
