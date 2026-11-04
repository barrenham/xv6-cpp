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
    assert '/user/fs/backends/' not in source, source
    assert not re.search(r'/kernel/fs/(ext3|devfs|procfs|ramfs)/', source), source
    assert not source.endswith('/kernel/fs/journal/journal.cpp'), source
    assert not re.search(r'/kernel/fs/(journal/flow|journal/xv6_journal|'
                         r'inode/xv6_filesystem|inode/disk_filesystem|inode/flow|'
                         r'path/path_operations|path/flow|buffer/snapshot_text)\.cppm?$', source), source


def symbols(image):
    return subprocess.check_output([args.nm, '--defined-only', '--demangle', str(image)], text=True)


kernel_symbols = symbols(args.kernel)
for name in ('Ext4FileSystem', 'Ext4Backend', 'userfs::ext4::Volume', 'Xv6FileSystem', 'Ext3FileSystem', 'DevFileSystem', 'ProcFileSystem',
             'RamFileSystem', 'DiskFileSystem', 'Xv6Journal', 'xv::ext3::'):
    assert name not in kernel_symbols, f'filesystem algorithm linked into kernel: {name}'
assert 'UserFileSystem' in kernel_symbols and 'create_mount_manager' in kernel_symbols
backend_types = {'ext3': 'Ext3Backend', 'ext4': 'Ext4Backend', 'xv6': 'Xv6Backend', 'ramfs': 'MemoryBackend',
                 'procfs': 'ProcessBackend', 'devfs': 'DeviceBackend'}
user_symbols = symbols(args.bootstrap)
for name in (*backend_types.values(), 'xv::ext3::Volume', 'userfs::xv6::Volume', 'userfs::ext4::Volume'):
    assert name not in user_symbols, f'filesystem algorithm linked into supervisor: {name}'
for backend, expected in backend_types.items():
    image = build / f'user/image/_{backend}_plugin'
    own_symbols = symbols(image)
    assert expected in own_symbols, f'{image}: backend module missing'
    for foreign in backend_types.values():
        if foreign != expected:
            assert foreign not in own_symbols, f'{image}: unrelated backend linked: {foreign}'
    compiled = [item for item in commands if f'user/fs/backends/{backend}/backend.cppm' in item['file']]
    assert compiled, f'{backend}: no actual C++ module interface compiled'

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
print(f'FS runtime boundary: {len(sources)} kernel sources checked; six separate backend ELFs, supervisor contains no backend; four launch targets use the generic boot bundle')
