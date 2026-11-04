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
        manifest = next(content for name, content, _ in assets if name == 'drivers/boot.list')
        assets = [asset for asset in assets if asset[0] != 'drivers/boot.list']
        commented = b'# RAM drivers\n' + b'\n'.join(b' ' + row for row in manifest.splitlines()) + b'\n'
        assets.append(('drivers/boot.list', commented, False))
        bundle = directory / 'bootstrap.elf'
        bundle.write_bytes(pack(base, assets))
        args.bootstrap = bundle
        root = directory / 'root.img'
        data = directory / 'data.img'
        shutil.copyfile(args.image, root)
        shutil.copyfile(args.image, data)
        guest = Guest(args, root, args.kernel.parent / f'boot-driver-{args.cpus}cpu.log',
                      data=data, network=True, extra=[
                          '-display', 'none',
                          '-device', 'virtio-gpu-device,bus=virtio-mmio-bus.3',
                          '-device', 'virtio-keyboard-device,bus=virtio-mmio-bus.4',
                          '-device', 'virtio-mouse-device,bus=virtio-mmio-bus.5'])
        try:
            login_root(guest.expect, guest.process)
            listing = guest.command('drvctl list', 'virtio_block bound=2')
            for name, abi in (('virtio_block', 1), ('uart16550', 2), ('virtio_rng', 2),
                              ('memory', 2), ('virtio_gpu', 2), ('virtio_input', 2), ('e1000', 2)):
                line = next(row for row in listing.splitlines() if f' {name} bound=' in row)
                assert f'abi={abi}' in line, line
                expected_bound = 2 if name in ('virtio_block', 'virtio_input') else 1
                assert f'bound={expected_bound} ' in line, line
                assert guest.output.index(f'driver: loaded {name} from boot archive'.encode()) < guest.output.index(b'fsboot:')
            guest.command('drvctl devices', 'bound=1')
            nodes = guest.command('ls /dev', 'vdb')
            for name in ('vda', 'vdb', 'random', 'urandom', 'null', 'zero', 'full',
                         'fb0', 'kbd0', 'mouse0'):
                assert name in nodes.split(), (name, nodes)
            guest.command('netctl dhcp wait eth0', 'eth0 DHCP BOUND')
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
