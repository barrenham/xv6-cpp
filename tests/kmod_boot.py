#!/usr/bin/env python3
"""Load a kernel driver from RAM before starting any user filesystem backend."""
import argparse
from pathlib import Path
import shutil
import sys
import tempfile

from console_login import login_root
from fs_elf_plugins import Guest, digest


def main():
    parser = argparse.ArgumentParser()
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(repo / 'tools'))
    from pack_boot_assets import pack, unpack
    original = {path: digest(path) for path in (args.kernel, args.bootstrap, args.image)}
    with tempfile.TemporaryDirectory(prefix='xv-boot-driver-') as temporary:
        directory = Path(temporary)
        base, assets = unpack(args.bootstrap.read_bytes())
        driver = args.kernel.parent / 'modules/virtio_block.elf'
        assets += [('drivers/boot.list', b'# RAM drivers\n drivers/vblk.elf # disk\n', False),
                   ('drivers/vblk.elf', driver.read_bytes(), False)]
        bundle = directory / 'bootstrap.elf'
        bundle.write_bytes(pack(base, assets))
        args.bootstrap = bundle
        root = directory / 'root.img'
        data = directory / 'data.img'
        shutil.copyfile(args.image, root)
        shutil.copyfile(args.image, data)
        guest = Guest(args, root, args.kernel.parent / f'boot-driver-{args.cpus}cpu.log', data=data)
        try:
            login_root(guest.expect, guest.process)
            guest.command('drvctl list', 'virtio_block bound=1')
            guest.command('drvctl devices', 'bound=1')
            guest.command('ls /dev', 'vdb')
            guest.command('mkdir /tmp/boot-disk')
            guest.command('mount -t ext3 -s 1 /tmp/boot-disk')
            guest.command('echo BOOT_DRIVER_READY > /tmp/boot-disk/marker')
            guest.command('sync')
            guest.command('umount /tmp/boot-disk')
            guest.command('mount -t ext3 -s 1 /tmp/boot-disk')
            guest.command('cat /tmp/boot-disk/marker', 'BOOT_DRIVER_READY')
            guest.command('umount /tmp/boot-disk')
            loaded = guest.output.index(b'driver: loaded virtio_block from boot archive')
            mounted = guest.output.index(b'fsboot:')
            assert loaded < mounted, 'driver initialization happened after user filesystem startup'
            assert guest.output.count(b'xv6 kernel is booting') == 1
        finally:
            guest.close()
    for path, checksum in original.items():
        assert digest(path) == checksum, f'changed source artifact: {path}'
    print(f'Boot driver {args.cpus} CPUs: RAM ELF loaded and bound before user FS, '
          'automatic device registration and persistent mount/write/remount passed')


if __name__ == '__main__':
    main()
