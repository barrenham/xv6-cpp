#!/usr/bin/env python3
"""Check concrete device algorithms reside in boot ELFs, not the kernel."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--kernel', type=Path, required=True)
parser.add_argument('--bootstrap', type=Path, required=True)
parser.add_argument('--nm', required=True)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / 'tools'))
from pack_boot_assets import unpack

commands = json.loads((args.kernel.parent / 'compile_commands.json').read_text())
kernel = [item for item in commands if 'kernel/CMakeFiles/kernel.dir/' in item['command']]
assert kernel
for item in kernel:
    source = Path(item['file']).resolve()
    assert not source.is_relative_to(repo / 'drivers'), f'device code compiled into kernel: {source}'
    assert 'virtio_block.cpp' not in source.name, source

def symbols(path):
    return subprocess.check_output([args.nm, '--defined-only', '--demangle', str(path)], text=True)

defined = symbols(args.kernel)
for implementation in ('VirtioBlock', 'VirtioGpu', 'VirtioKeyboard', 'VirtioEntropy',
                       'Uart16550', 'E1000', 'MemoryDevice', 'configure_e1000'):
    assert implementation not in defined, f'concrete device linked into kernel: {implementation}'
assert 'ModuleBlock' in defined and 'CharacterProxy' in defined and 'SerialProxy' in defined

_, assets = unpack(args.bootstrap.read_bytes())
assets = {name: data for name, data, _ in assets}
manifest = assets['drivers/boot.list'].decode()
entries = [row.split('#', 1)[0].strip().split()[0] for row in manifest.splitlines()
           if row.split('#', 1)[0].strip()]
expected = {
    'vblk': args.kernel.parent / 'modules/virtio_block.elf',
    **{name: args.kernel.parent / f'drivers/{name}.elf'
       for name in ('serial', 'rng', 'mem', 'gpu', 'input', 'e1000')},
}
assert set(entries) == {f'drivers/{name}.elf' for name in expected}, entries
for name, path in expected.items():
    data = path.read_bytes()
    assert assets[f'drivers/{name}.elf'] == data, f'{name}: boot archive carries stale code'
    assert data[:7] == b'\x7fELF\x02\x01\x01'
    assert struct.unpack_from('<HH', data, 16) == (3, 243), name
    assert 'xv_driver_entry' in symbols(path), name
    undefined = subprocess.check_output([args.nm, '--undefined-only', str(path)], text=True)
    assert not undefined.strip(), f'{name}: imports unresolved kernel symbols: {undefined}'
    assert len(data) <= 256 * 1024, f'{name}: exceeds loader bound'

print(f'Device ELF boundary: {len(kernel)} kernel inputs checked; seven independent '
      'ELFs byte-match the automatic boot manifest; no concrete driver linked into kernel')
