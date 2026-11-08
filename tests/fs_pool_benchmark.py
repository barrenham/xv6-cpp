#!/usr/bin/env python3
"""Measure an archived or current FS build using private disks, without perf gates."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for name in ('kernel', 'bootstrap', 'image'):
    parser.add_argument('--' + name, required=True, type=Path)
parser.add_argument('--qemu', default='qemu-system-riscv64')
parser.add_argument('--cpus', type=int, choices=(1, 5), required=True)
parser.add_argument('--label', default='thread_pool')
parser.add_argument('--runs', type=int, default=2)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
if not 1 <= args.runs <= 10:
    parser.error('--runs must be 1..10')
records = []
with tempfile.TemporaryDirectory(prefix='xv-fs-pool-benchmark-') as temporary:
    image = Path(temporary) / 'root.img'
    shutil.copyfile(args.image, image)
    guest = Guest(args, image, args.kernel.parent / f'fs-pool-benchmark-{args.cpus}cpu.log')
    try:
        login_root(guest.expect, guest.process)
        for run in range(args.runs):
            output = guest.command('/usr/tests/fspoolbench', ' PASS', timeout=180)
            match = re.search(r'fspoolbench: (.*?) PASS', output)
            if not match:
                raise AssertionError(output)
            record = dict(item.split('=') for item in match.group(1).split())
            record = {key: int(value) for key, value in record.items()}
            assert record['ops'] == 1024 and record['bytes'] == 4194304, record
            record.update(label=args.label, cpus=args.cpus, run=run,
                          cpu_ms=record['cpu_ticks'] * 10,
                          bytes_per_second=record['bytes'] * 1000000 // record['elapsed_us'])
            records.append(record)
            print(json.dumps(record, sort_keys=True), flush=True)
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps({
    'kernel_sha256': hashlib.sha256(args.kernel.read_bytes()).hexdigest(),
    'image_sha256': hashlib.sha256(args.image.read_bytes()).hexdigest(),
    'records': records,
    'notes': '2 forked clients, separate 512 KiB files; 256 write/read pairs each. '
             'CPU is whole-system /proc/stat in 10 ms units. Memory is global '
             'MemFree delta after mount and seeding, including caches and file storage. '
             'Run 0 and later runs have different cache warmth; not a performance gate.'
}, indent=2) + '\n')
