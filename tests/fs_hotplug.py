#!/usr/bin/env python3
"""Compile a new filesystem ELF AFTER boot and load it into that same running kernel."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
from console_login import login_root
from fs_elf_plugins import Guest, digest
from fs_live_install import Server, install
from ext4_fixtures import make_image, command


def main():
    parser = argparse.ArgumentParser()
    for name in ('kernel', 'bootstrap', 'image', 'executable'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--cpus', required=True, type=int)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(repo / 'tools'))
    from export_fs_driver_kit import export
    from create_rootfs import Image
    from update_rootfs import lookup
    from pack_boot_assets import unpack
    original = {path: digest(path) for path in (args.kernel, args.bootstrap, args.image)}
    base, assets = unpack(args.bootstrap.read_bytes())
    assert not any('ext4' in name for name, _, _ in assets), 'driver already bundled before boot'
    with tempfile.TemporaryDirectory(prefix='xv-live-driver-') as temporary:
        directory = Path(temporary)
        data = make_image(directory, 1024, executable=args.executable)
        data_hash = digest(data)
        root = directory / 'root.img'; shutil.copyfile(args.image, root)
        baseline = Image(root)
        for path in ('/usr/lib/fsplugins', '/etc/fsplugins'):
            names = baseline.entries(lookup(baseline, Path(path).parts[1:]))
            assert not any(b'ext4' in name for name in names), 'driver preinstalled before boot'
        del baseline
        guest = Guest(args, root, args.kernel.parent / f'hot-filesystem-{args.cpus}cpu.log', data=data, network=True)
        server = None
        try:
            login_root(guest.expect, guest.process)
            guest.command('drvctl --raw list', 'virtio_block bound=2')
            guest.command('cd /')
            guest.command('mkdir /tmp/newfs')
            guest.command('mount -t hotext4 /tmp/newfs', failure=True)
            running_pid = guest.process.pid
            # No project, source copy, object, ELF, or manifest exists before the guest shell is ready.
            started = time.time_ns()
            kit = export(repo, directory / 'new-driver-kit')
            project = kit / 'examples/fs-driver'
            build = directory / 'new-driver-build'
            subprocess.run([args.cmake, '-S', str(project), '-B', str(build), '-G', 'Ninja',
                            f'-DCMAKE_TOOLCHAIN_FILE={project / "toolchain.cmake"}',
                            f'-DXVFS_KIT_ROOT={kit}', '-DXVFS_DRIVER_NAME=hotext4'], check=True)
            subprocess.run([args.cmake, '--build', str(build), '-j', '1'], check=True)
            assert guest.process.pid == running_pid and guest.process.poll() is None
            binary = build / 'hotext4.elf'
            assert binary.stat().st_mtime_ns >= started
            commands = json.loads((build / 'compile_commands.json').read_text())
            assert commands and not any('/kernel/' in item['file'] for item in commands), 'kernel source compiled'
            server = Server(build)
            install(guest, server, 'hotext4')
            guest.command('mount -t hotext4 /tmp/newfs')
            guest.command('fsctl /tmp/newfs', 'Interface       : ext4.native')
            guest.command('ext4test /tmp/newfs', 'ext4test: PASS:', timeout=100)
            guest.command('/tmp/newfs/probe', 'cppdemo: RAII and move ownership OK')
            guest.command('umount /tmp/newfs')
            guest.command('mount -t hotext4 /tmp/newfs')
            guest.command('cat /tmp/newfs/message', 'EXT4_READ_ONLY_OKAY')
            guest.command('umount /tmp/newfs')
            guest.command('sync')
            assert guest.process.pid == running_pid and guest.process.poll() is None
            assert guest.output.count(b'xv6 kernel is booting') == 1, 'guest restarted'
        finally:
            guest.close()
            if server: server.close()
        installed = Image(root)
        node = lookup(installed, ('usr', 'lib', 'fsplugins', 'hotext4.elf'))
        assert installed.read(node) == binary.read_bytes(), 'live ELF transfer changed bytes'
        assert digest(data) == data_hash, 'read-only data disk changed'
        command([shutil.which('e2fsck') or '/usr/sbin/e2fsck', '-fn', str(data)])
    for path, value in original.items():
        assert digest(path) == value, f'hot load changed preboot artifact {path}'
    print(f'Live FS ({args.cpus} CPUs): absent before boot, source/ELF created after login, '
          'downloaded/loaded/mounted/read/executed without restarting QEMU or rebuilding kernel/bootstrap')


if __name__ == '__main__':
    main()
