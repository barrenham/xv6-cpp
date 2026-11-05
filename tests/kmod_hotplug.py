#!/usr/bin/env python3
"""Build a real kernel driver after boot, load/bind it, and persist FS writes."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import struct
import tempfile
import time
import sys

from console_login import login_root
from fs_elf_plugins import Guest, digest
from fs_live_install import Server


def invalid_images(binary, directory):
    original = binary.read_bytes()
    def put(name, change):
        data = bytearray(original)
        change(data)
        (directory / (name + '.elf')).write_bytes(data)
        return name
    variants = [put('wrong-machine', lambda b: struct.pack_into('<H', b, 18, 62)),
                put('bad-entry', lambda b: struct.pack_into('<Q', b, 24, 0xffff0000)),
                put('bad-abi-flags', lambda b: struct.pack_into('<I', b, 48, 0xffff))]
    phoff = struct.unpack_from('<Q', original, 32)[0]
    phnum = struct.unpack_from('<H', original, 56)[0]
    headers = [struct.unpack_from('<IIQQQQQQ', original, phoff + i * 56) for i in range(phnum)]
    text = next(i for i, p in enumerate(headers) if p[0] == 1 and p[1] & 1)
    variants.append(put('rwx', lambda b: struct.pack_into('<I', b, phoff + text * 56 + 4, 7)))
    dynamic = next(p for p in headers if p[0] == 2)
    tags = dict(struct.unpack_from('<qQ', original, at)
                for at in range(dynamic[2], dynamic[2] + dynamic[5], 16))
    rela_va = tags[7]
    segment = next(p for p in headers if p[0] == 1 and p[3] <= rela_va < p[3] + p[5])
    rela_file = segment[2] + rela_va - segment[3]
    variants.append(put('unsupported-relocation', lambda b: struct.pack_into('<Q', b, rela_file + 8, 2)))
    variants.append(put('text-relocation', lambda b: struct.pack_into('<Q', b, rela_file, headers[text][3])))
    signature = struct.pack('<IIII', 56, 1, 1, 2) + b'hotdisk\0'
    descriptor = original.index(signature)
    variants.append(put('wrong-driver-abi', lambda b: struct.pack_into('<I', b, descriptor + 4, 99)))
    return variants


def main():
    parser = argparse.ArgumentParser()
    for name in ('kernel', 'bootstrap', 'image', 'mkfs'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--cmake', default='cmake')
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    originals = {p: digest(p) for p in (args.kernel, args.bootstrap, args.image)}
    with tempfile.TemporaryDirectory(prefix='xv-kernel-driver-') as temporary:
        directory = Path(temporary)
        sys.path.insert(0, str(repo / 'tools'))
        from pack_boot_assets import pack, unpack
        base, assets = unpack(args.bootstrap.read_bytes())
        boot_manifest = next(content for name, content, _ in assets if name == 'drivers/boot.list')
        assets = [a for a in assets if a[0] != 'drivers/boot.list']
        boot_manifest = boot_manifest.replace(b'drivers/vblk.elf\n', b'drivers/vblk.elf 0\n')
        assets.append(('drivers/boot.list', boot_manifest, False))
        args.bootstrap = directory / 'root-only.elf'
        args.bootstrap.write_bytes(pack(base, assets))
        root = directory / 'root.img'
        shutil.copyfile(args.image, root)
        data = directory / 'data.img'
        seed = directory / 'message'
        seed.write_text('KMOD_DISK_INITIAL_CONTENT\n')
        manifest = directory / 'manifest.tsv'
        manifest.write_text(f'file\t0644\t/message\t{seed}\n')
        subprocess.run([str(args.mkfs), str(data), '--manifest', str(manifest),
                        '--filesystem', 'ext3'], check=True)
        guest = Guest(args, root, args.kernel.parent / f'kernel-driver-{args.cpus}cpu.log',
                      data=data, network=True)
        server = None
        try:
            login_root(guest.expect, guest.process)
            guest.command('cd /')
            baseline = guest.command('drvctl --raw list')
            count = int(re.search(r'(\d+) resident modules', baseline)[1])
            guest.command('drvctl --raw devices', 'bound=0')
            assert 'vdb' not in guest.command('ls /dev'), 'disk registered before loading driver'
            guest.command('mkdir /tmp/module-disk')
            guest.command('mount -t ext3 -s 1 /tmp/module-disk', failure=True)
            pid = guest.process.pid
            # Copy just the public ABI and the standalone example after login.
            started = time.time_ns()
            source = directory / 'new-driver'
            shutil.copytree(repo / 'examples/kernel-driver', source)
            shutil.copyfile(repo / 'kernel/io/kmodule.h', source / 'kmodule.h')
            # SATP is supervisor-only. This probe would trap if the driver were
            # executed in a user process rather than by the kernel loader.
            driver_source = source / 'virtio_block.cpp'
            code = driver_source.read_text()
            code = code.replace('  s->host = host;',
                '  unsigned long satp;\n'
                '  asm volatile("csrr %0, satp" : "=r"(satp));\n'
                '  if (!satp)\n'
                '  {\n'
                '    return -1;\n'
                '  }\n'
                '  s->host = host;')
            driver_source.write_text(code)
            build = directory / 'new-build'
            subprocess.run([args.cmake, '-S', str(source), '-B', str(build), '-G', 'Ninja',
                            f'-DXVK_ABI_DIR={source}', '-DXVK_NAME=hotdisk'], check=True)
            subprocess.run([args.cmake, '--build', str(build), '-j', '1'], check=True)
            failed_source = source / 'fail.cpp'
            failed_source.write_text(code.replace('  *out = {',
                '  write(*s, 0x70, 0);\n  return -1;\n  *out = {'))
            failed_build = directory / 'failed-build'
            subprocess.run([args.cmake, '-S', str(source), '-B', str(failed_build), '-G', 'Ninja',
                            f'-DXVK_ABI_DIR={source}', '-DXVK_NAME=faildisk',
                            f'-DXVK_SOURCE={failed_source}'], check=True)
            subprocess.run([args.cmake, '--build', str(failed_build), '-j', '1'], check=True)
            shutil.copyfile(failed_build / 'faildisk.elf', build / 'faildisk.elf')
            binary = build / 'hotdisk.elf'
            assert binary.stat().st_mtime_ns >= started
            invalid = invalid_images(binary, build)
            server = Server(build)
            guest.command(f'curl -fsS -m 30 -o /tmp/hotdisk.elf {server.url}/hotdisk.elf', timeout=45)
            guest.command('drvctl load --no-bind /bin/echo', 'load failed')
            for name in invalid:
                guest.command(f'curl -fsS -m 30 -o /tmp/bad.elf {server.url}/{name}.elf', timeout=45)
                guest.command('drvctl load --no-bind /tmp/bad.elf', 'load failed')
            guest.command('drvctl --raw list', f'{count} resident modules')
            guest.command('drvctl load --no-bind /tmp/hotdisk.elf', f'loaded module {count}')
            guest.command('drvctl --raw list', 'hotdisk bound=0')
            guest.command('drvctl load --no-bind /tmp/hotdisk.elf', 'load failed')
            guest.command(f'drvctl bind {count} 0', 'bind failed')
            guest.command(f'curl -fsS -m 30 -o /tmp/faildisk.elf {server.url}/faildisk.elf', timeout=45)
            guest.command('drvctl load --no-bind /tmp/faildisk.elf', f'loaded module {count + 1}')
            guest.command(f'kmodtest --failed-bind {count + 1} 1', 'failed bind retries reclaim resources')
            guest.command(f'drvctl bind {count} 1', f'bound module {count} to device 1')
            guest.command(f'drvctl bind {count} 1', 'bind failed')
            guest.command('drvctl --raw devices', 'bound=1')
            guest.command('ls /dev', 'vdb')
            guest.command('kmodtest', 'kmodtest: PASS:', timeout=80)
            guest.command('mount -t ext3 -s 1 /tmp/module-disk')
            guest.command('cat /tmp/module-disk/message', 'KMOD_DISK_INITIAL_CONTENT')
            guest.command('echo KERNEL_MODULE_PERSISTED > /tmp/module-disk/written')
            guest.command('sync')
            guest.command('umount /tmp/module-disk')
            guest.command('mount -t ext3 -s 1 /tmp/module-disk')
            guest.command('cat /tmp/module-disk/written', 'KERNEL_MODULE_PERSISTED')
            guest.command('umount /tmp/module-disk')
            stats = guest.command('drvctl --raw list', 'hotdisk bound=1')
            matched = re.search(rf'{count} hotdisk bound=1 transfers=(\d+) interrupts=(\d+)', stats)
            assert matched and int(matched[1]) > 0 and int(matched[2]) > 0, stats
            assert 'hotdisk' not in guest.command('ps'), 'driver incorrectly runs as a user process'
            assert guest.process.pid == pid and guest.process.poll() is None
            assert guest.output.count(b'xv6 kernel is booting') == 1
        finally:
            guest.close()
            if server: server.close()
        # Verify persistence independently of the guest VFS and block cache.
        subprocess.run(['debugfs', '-R', 'cat /written', str(data)],
                       check=True, capture_output=True).stdout.index(b'KERNEL_MODULE_PERSISTED')
        subprocess.run(['e2fsck', '-fn', str(data)], check=True)
    for path, checksum in originals.items():
        assert digest(path) == checksum, f'changed preboot artifact {path}'
    print(f'Kernel driver {args.cpus} CPUs: post-boot independent ELF compilation, load/bind, '
          'kernel callbacks/IRQs, userspace FS mount and persistent writes passed')


if __name__ == '__main__':
    main()
