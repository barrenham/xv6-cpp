#!/usr/bin/env python3
"""Verify the production kernel contains mechanisms and user ELF contains FS backends."""
import argparse
import json
from pathlib import Path
import re
import shlex
import subprocess


parser = argparse.ArgumentParser()
parser.add_argument('--kernel', type=Path, required=True)
parser.add_argument('--bootstrap', type=Path, required=True)
parser.add_argument('--nm', required=True)
parser.add_argument('--ninja', required=True)
args = parser.parse_args()
build = args.kernel.resolve().parent
commands = json.loads((build / 'compile_commands.json').read_text())
sources = [item['file'] for item in commands if 'kernel/CMakeFiles/kernel.dir/' in item['command']]
assert sources, 'no kernel compilation records inspected'
for source in sources:
    assert not re.search(r'/kernel/fs/(ext3|devfs|procfs|ramfs)/', source), source
    assert not source.endswith('/kernel/fs/journal/journal.cpp'), source
    assert not re.search(r'/kernel/fs/(journal/flow|journal/xv6_journal|'
                         r'inode/xv6_filesystem|inode/disk_filesystem|inode/flow|'
                         r'path/path_operations|path/flow|buffer/snapshot_text)\.cppm?$', source), source


def symbols(image):
    return subprocess.check_output([args.nm, '--defined-only', '--demangle', str(image)], text=True)


kernel_symbols = symbols(args.kernel)
for name in ('Xv6FileSystem', 'Ext3FileSystem', 'DevFileSystem', 'ProcFileSystem',
             'RamFileSystem', 'DiskFileSystem', 'Xv6Journal', 'xv::ext3::'):
    assert name not in kernel_symbols, f'filesystem algorithm linked into kernel: {name}'
assert 'UserFileSystem' in kernel_symbols and 'create_mount_manager' in kernel_symbols
user_symbols = symbols(args.bootstrap)
for name in ('Ext3Backend', 'Xv6Backend', 'MemoryBackend', 'ProcessBackend', 'DeviceBackend'):
    assert name in user_symbols, f'bootstrap lacks userspace backend: {name}'

for target in ('qemu', 'qemu-vnc', 'qemu-graphics', 'qemu-gdb'):
    output = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'commands', target], text=True)
    boots = [shlex.split(line) for line in output.splitlines() if ' -kernel ' in line]
    assert len(boots) == 1, f'{target}: expected one QEMU launch, got {len(boots)}'
    boot = boots[0]
    assert boot.count('-kernel') == 1 and boot.count('-initrd') == 1, target
    payload = Path(boot[boot.index('-initrd') + 1])
    if not payload.is_absolute():
        payload = build / payload
    assert payload.resolve() == args.bootstrap.resolve(), (target, payload)
print(f'FS runtime boundary: {len(sources)} kernel sources checked; all five backends in userspace; four default launch targets use bootstrap')
