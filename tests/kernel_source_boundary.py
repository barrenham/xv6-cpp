#!/usr/bin/env python3
"""Keep obsolete backend implementations out of the kernel source tree."""
import argparse
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, required=True)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[1]
records = json.loads((args.build / 'compile_commands.json').read_text())
compiled = {Path(row['file']).resolve() for row in records
            if 'kernel/CMakeFiles/kernel.dir/' in row['command']}
assert compiled, 'missing production kernel compilation records'
sources = {path.resolve() for path in (repo / 'kernel').rglob('*')
           if path.suffix in ('.cpp', '.cppm', '.c', '.S')}
assert sources <= compiled, f'nonproduction source retained in kernel: {sorted(sources - compiled)}'
for directory in ('ext3', 'devfs', 'procfs', 'ramfs', 'journal', 'path', 'format'):
    assert not any((repo / 'kernel/fs' / directory).rglob('*')), directory
for relative in ('kernel/fs/inode/xv6_filesystem.cpp', 'kernel/fs/inode/disk_filesystem.cpp',
                 'kernel/fs/vfs/initialize_flow.cppm'):
    assert not (repo / relative).exists(), relative
for name in ('volume.cpp', 'mutate.cpp', 'jbd.cpp', 'volume.h', 'jbd.h', 'format.h'):
    assert (repo / 'user/fs/backends/ext3' / name).is_file(), name
assert (repo / 'user/fs/backends/xv6/format.h').is_file()
text = '\n'.join(path.read_text() for path in sources if path.suffix != '.S')
for obsolete in ('ServiceJournal', 'Transaction::secondary', 'service_mode_',
                 'VirtualFileSystem::mount_ram', 'VirtualFileSystem::mount_disk'):
    assert obsolete not in text, obsolete
print(f'Kernel source boundary: all {len(sources)} sources belong to the production kernel; '
      'old FS implementations and mode/journal fallbacks removed')
