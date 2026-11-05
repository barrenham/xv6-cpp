#!/usr/bin/env python3
"""Measure fixed-size metadata/read/write workloads without changing persistent disks."""
import argparse
import json
from pathlib import Path
import re
import shutil
import sys
import tempfile
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image', 'output'):
    parser.add_argument('--' + option, type=Path, required=True)
parser.add_argument('--qemu', default='qemu-system-riscv64')
parser.add_argument('--cpus', type=int, default=5)
args = parser.parse_args()
rows = []
pattern = re.compile(r'FSPAR phase=(\w+) workers=(\d+) operations=(\d+) ticks=(\d+) rpc=(\d+) copied=(\d+) waits=(\d+) payload=(\d+)')
with tempfile.TemporaryDirectory(prefix='xv-fs-concurrency-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    guest = Guest(args, image, args.output.with_suffix('.guest.log'))
    try:
        login_root(guest.expect, guest.process)
        for backend, parent in (('ramfs', '/tmp'), ('ext3', '/root')):
            for workers in (1, 4):
                output = guest.command(f'/usr/tests/fsparallelbench {parent}/fspar {workers}',
                                       'fsparallelbench: PASS', timeout=180)
                print(output, flush=True)
                matches = pattern.findall(output)
                if len(matches) != 4:
                    raise AssertionError('benchmark did not report all phases')
                for phase, *values in matches:
                    row = dict(zip(('workers', 'operations', 'ticks', 'rpc', 'copied', 'waits', 'payload'),
                                   map(int, values)))
                    row.update(backend=backend, phase=phase, cpus=args.cpus)
                    rows.append(row)
        guest.command('sync')
    finally:
        guest.close()
args.output.write_text(json.dumps({'timebase_hz': 10000000, 'rows': rows}, indent=2) + '\n')
