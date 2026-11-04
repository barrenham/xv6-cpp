#!/usr/bin/env python3
"""Boot an unknown root plugin on an unformatted disk, with unchanged kernel/launcher."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from fs_elf_plugins import Guest, digest


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', required=True, type=Path)
    parser.add_argument('--bootstrap', required=True, type=Path)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--example', required=True, type=Path)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--cpus', required=True, type=int)
    args = parser.parse_args()
    for name in ('kernel', 'bootstrap', 'sdk', 'example'):
        setattr(args, name, getattr(args, name).resolve())
    unchanged = {path: digest(path) for path in (args.kernel, args.bootstrap)}
    repo = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(repo / 'tools'))
    from pack_boot_assets import pack, unpack
    _, boot_assets = unpack(args.bootstrap.read_bytes())
    drivers = [asset for asset in boot_assets if asset[0].startswith("drivers/")]
    with tempfile.TemporaryDirectory(prefix='xv-external-root-') as temporary:
        directory = Path(temporary)
        sdk = directory / 'sdk'
        sdk.mkdir()
        for name in ('include', 'standalone'):
            shutil.copytree(args.sdk / name, sdk / name)
        source = directory / 'source'
        shutil.copytree(args.example, source)

        def compile(name, fixture):
            target = source / f'{name}.c'
            shutil.copyfile(repo / 'tests/fixtures' / fixture, target)
            build = directory / name
            subprocess.run([args.cmake, '-S', str(source), '-B', str(build), '-G', 'Ninja',
                            f'-DXVFS_SDK={sdk}', f'-DXVFS_PLUGIN_SOURCE={target}',
                            f'-DCMAKE_TOOLCHAIN_FILE={sdk / "standalone/riscv64-toolchain.cmake"}'], check=True)
            subprocess.run([args.cmake, '--build', str(build), '-j', '1'], check=True)
            return (build / 'hello.elf').read_bytes()

        guard = compile('guard', 'fs_asset_guard.c')
        guard_bundle = directory / 'asset-guard.elf'
        guard_bundle.write_bytes(pack(guard, [('data', b'DATA', False),
                                              ('bad', b'not ELF', True), ('self', guard, True)] + drivers))
        default_bundle = args.bootstrap
        args.bootstrap = guard_bundle
        disk = directory / 'guard.img'
        disk.write_bytes(bytes(256 * 1024))
        guest = Guest(args, disk, args.kernel.parent / f'asset-guard-{args.cpus}cpu.log')
        try:
            guest.expect('asset-guard: PASS: bounds, failed exec, immutable descriptors and delegated identity')
        finally:
            guest.close()
        args.bootstrap = default_bundle
        init = compile('init', 'fs_plugin_init.c')
        (source / 'init_image.h').write_text('static const unsigned char init_image[] = {' +
                                           ','.join(str(byte) for byte in init) + '};\n')
        root = compile('root', 'fs_root_plugin.c')
        base, assets = unpack(args.bootstrap.read_bytes())
        merged = {name: (name, data, executable) for name, data, executable in assets}
        config = merged['boot.cfg'][1].replace(b'root=auto', b'root=modroot')
        merged['boot.cfg'] = ('boot.cfg', config, False)
        manifest = (b'abi=1\ntype=modroot\nexec=/arbitrary/root.elf\n'
                    b'image=fsplugins/modroot.elf\nresource=none\n')
        merged['fsplugins/modroot.conf'] = ('fsplugins/modroot.conf', manifest, False)
        merged['fsplugins/modroot.elf'] = ('fsplugins/modroot.elf', root, True)
        custom = directory / 'custom-boot.elf'
        custom.write_bytes(pack(base, list(merged.values())))
        args.bootstrap = custom
        disk = directory / 'unformatted.img'
        disk.write_bytes(bytes(256 * 1024))
        before = hashlib.sha256(disk.read_bytes()).digest()
        guest = Guest(args, disk, args.kernel.parent / f'external-root-{args.cpus}cpu.log')
        try:
            guest.expect('external-root: independently loaded init and procfs passed', timeout=60)
            for kind, path in (('modroot', '/'), ('devfs', '/dev'), ('proc', '/proc'), ('ramfs', '/tmp')):
                assert f'fsboot: {kind} mounted at {path}, server pid='.encode() in guest.output
        finally:
            guest.close()
        assert hashlib.sha256(disk.read_bytes()).digest() == before, 'root plugin touched the unused boot disk'
    for path, value in unchanged.items():
        assert digest(path) == value, f'external root plugin rebuilt {path}'
    print(f'External root ({args.cpus} CPUs): unknown C ELF served init on an unformatted disk; '
          'kernel and supervisor unchanged; independent core plugins active')


if __name__ == '__main__':
    main()
