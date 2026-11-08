#!/usr/bin/env python3
"""Prove shared inode reads, shared offsets and independent in-flight FS RPCs."""
import argparse
from pathlib import Path
import re
import shutil
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image'):
    parser.add_argument('--' + option, type=Path, required=True)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', type=int, required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-fs-concurrency-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    guest = Guest(args, image, args.kernel.parent / f'fs-concurrency-{args.cpus}cpu.log')
    try:
        login_root(guest.expect, guest.process)
        print(guest.command('/usr/tests/fsconcurtest',
                            'fsconcurrencytest: PASS: same/distinct inode readers, shared offsets',
                            timeout=45))
        for path in ('/tmp/fscache', '/root/fscache'):
            print(guest.command('/usr/tests/fscachetest ' + path, 'fscachetest: PASS', timeout=90))
        for path in ('/tmp/fspar', '/root/fspar'):
            output = guest.command('/usr/tests/fsparbench ' + path + ' 4',
                                   'fsparallelbench: PASS', timeout=120)
            print(output)
            assert len(re.findall(r'^FSPAR phase=', output, re.M)) == 4, output
            assert 'FSPAR_LANES lanes=3 ' in output, output
        guest.command('sync')
    finally:
        guest.close()
print('FS concurrency: same/distinct inode reads, shared offsets, reverse replies, disconnect and SDK lanes PASS')
