#!/usr/bin/env python3
"""Use formerly unmapped DT slots and automatically bind a runtime driver."""
import argparse
from pathlib import Path
import re
import shutil
import sys
import tempfile
from types import SimpleNamespace
from console_login import login_root
from fs_elf_plugins import Guest
from fs_live_install import Server

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image', 'module'):
    parser.add_argument('--' + option, required=True, type=Path)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', required=True, type=int)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / 'tools'))
from pack_boot_assets import pack, unpack

with tempfile.TemporaryDirectory(prefix='xv-dt-resources-') as temporary:
    directory = Path(temporary)
    root = directory / 'high-slots.img'
    shutil.copyfile(args.image, root)
    guest = Guest(args, root, args.kernel.parent / f'dt-high-slots-{args.cpus}cpu.log',
                  root_bus=6, rng_bus=7, network=True)
    try:
        login_root(guest.expect, guest.process)
        listing = guest.command('drvctl --raw devices')
        assert re.search(r'^6 virtio-mmio.6 bus=1 device=2 irq=7 bound=1', listing, re.M), listing
        assert re.search(r'^7 virtio-mmio.7 bus=1 device=4 irq=8 bound=1', listing, re.M), listing
        assert 'bus=4' in listing and 'device=4110' in listing, listing
        guest.command('echo DT_SLOT_IO_OK > /root/dt-proof')
        guest.command('cat /root/dt-proof', 'DT_SLOT_IO_OK')
        guest.command('drvctl list', 'virtio_block')
        guest.command('sync')
    finally:
        guest.close()

    base, assets = unpack(args.bootstrap.read_bytes())
    assets = [(name, contents.replace(b'drivers/vblk.elf\n', b'drivers/vblk.elf 0\n')
               if name == 'drivers/boot.list' else contents, executable)
              for name, contents, executable in assets]
    bootstrap = directory / 'manual-root.elf'
    bootstrap.write_bytes(pack(base, assets))
    runtime_args = SimpleNamespace(**vars(args))
    runtime_args.bootstrap = bootstrap
    root = directory / 'auto-root.img'
    data = directory / 'auto-data.img'
    shutil.copyfile(args.image, root)
    shutil.copyfile(args.image, data)
    binary = args.module.read_bytes()
    old = b'virtio_block' + b'\0' * (32 - len('virtio_block'))
    assert binary.count(old) == 1, 'expected one ABI descriptor name'
    (directory / 'autodisk.elf').write_bytes(binary.replace(old, b'autodisk' + b'\0' * 24))
    guest = Guest(runtime_args, root, args.kernel.parent / f'driver-auto-{args.cpus}cpu.log',
                  data=data, network=True)
    server = Server(directory)
    try:
        login_root(guest.expect, guest.process)
        guest.command('drvctl --raw devices', 'device=2 irq=2 bound=0')
        listing = guest.command('drvctl --raw list')
        module = int(re.search(r'(\d+) resident modules', listing)[1])
        assert 'vdb' not in guest.command('ls /dev')
        guest.command(f'curl -fsS -m 30 -o /tmp/autodisk.elf {server.url}/autodisk.elf')
        guest.command('drvctl load /tmp/autodisk.elf', 'automatically bound 1 devices')
        guest.command('drvctl --raw list', 'autodisk bound=1')
        guest.command('drvctl --raw devices', 'device=2 irq=2 bound=1')
        guest.command('ls /dev', 'vdb')
        guest.command(f'drvctl bind --auto {module}', 'automatically bound 0 devices')
        guest.command('mkdir /tmp/auto-volume')
        guest.command('mount -t ext3 -s 1 /tmp/auto-volume')
        guest.command('echo AUTO_BIND_IO_OK > /tmp/auto-volume/auto-proof')
        guest.command('cat /tmp/auto-volume/auto-proof', 'AUTO_BIND_IO_OK')
        guest.command('umount /tmp/auto-volume')
        guest.command('sync')
    finally:
        guest.close()
        server.close()
print('DT high-slot discovery and automatic runtime driver binding: PASS')
