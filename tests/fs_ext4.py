#!/usr/bin/env python3
"""Mount the independent ext4 ELF against an e2fsprogs disk in QEMU."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import tempfile
from console_login import login_root
from fs_elf_plugins import Guest, digest
from ext4_fixtures import make_image, command
from fs_live_install import Server, install


def main():
    parser = argparse.ArgumentParser()
    for name in ('kernel', 'bootstrap', 'image', 'executable'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--cpus', type=int, required=True)
    parser.add_argument('--block', type=int, default=1024)
    args = parser.parse_args()
    unchanged = {path: digest(path) for path in (args.kernel, args.bootstrap, args.image)}
    with tempfile.TemporaryDirectory(prefix='xv-ext4-guest-') as temporary:
        directory = Path(temporary)
        data = make_image(directory, args.block, executable=args.executable)
        before = digest(data)
        root = directory / 'root.img'; shutil.copyfile(args.image, root)
        plugin = args.kernel.parent / 'user/image/_ext4_plugin'
        shutil.copyfile(plugin, directory / 'ext4.elf')
        (directory / 'ext4.conf').write_text('abi=1\ntype=ext4\nexec=/usr/lib/fsplugins/ext4.elf\nresource=block\nsource=1\n')
        server = Server(directory)
        guest = Guest(args, root, args.kernel.parent / f'ext4-{args.block}-{args.cpus}cpu.log', data=data, network=True)
        try:
            login_root(guest.expect, guest.process)
            guest.command('drvctl --raw list', 'virtio_block bound=2')
            guest.command('cd /')
            guest.command('mkdir /tmp/ext4')
            guest.command('mount -t ext4 /tmp/ext4', failure=True)
            install(guest, server, 'ext4')
            guest.command('mount -t ext4 /tmp/ext4')
            guest.command('fsctl /tmp/ext4', 'Interface       : ext4.native')
            guest.command('ext4test /tmp/ext4', 'ext4test: PASS:', timeout=100)
            guest.command('/tmp/ext4/probe', 'cppdemo: RAII and move ownership OK')
            guest.command('umount /tmp/ext4')
            for _ in range(4):
                guest.command('mount -t ext4 /tmp/ext4')
                guest.command('cat /tmp/ext4/message', 'EXT4_READ_ONLY_OKAY')
                guest.command('umount /tmp/ext4')
        finally:
            guest.close()
            server.close()
        assert digest(data) == before, 'readonly driver modified ext4 disk'
        command([shutil.which('e2fsck') or '/usr/sbin/e2fsck', '-fn', str(data)])
    for path, value in unchanged.items():
        assert digest(path) == value, f'test changed {path}'
    print(f'ext4 {args.block} bytes/{args.cpus} CPUs: real disk mount/read/exec, repeated lifecycle and byte-identical disk passed')


if __name__ == '__main__':
    main()
