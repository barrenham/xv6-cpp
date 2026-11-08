#!/usr/bin/env python3
"""Exercise real V3 ELF loading, dependency order and deferred runtime binding."""
import argparse
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tempfile

from console_login import login_root
from fs_elf_plugins import Guest, digest
from fs_live_install import Server

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from pack_boot_assets import pack, unpack


def test_devices(blob):
    """Add described test resources; attach callbacks never access their MMIO."""
    words = list(struct.unpack_from('>10I', blob))
    structure = blob[words[2]:words[2] + words[9]]
    strings = bytearray(blob[words[3]:words[3] + words[8]])
    at = 0
    stack = []
    plic = None
    insertion = None
    while at < len(structure):
        start = at
        token, = struct.unpack_from('>I', structure, at)
        at += 4
        if token == 1:
            end = structure.index(0, at)
            stack.append({})
            at = (end + 4) & ~3
        elif token == 2:
            node = stack.pop()
            if b'riscv,plic0\0' in node.get('compatible', b''):
                plic = node['phandle']
            if not stack:
                insertion = start
        elif token == 3:
            count, offset = struct.unpack_from('>II', structure, at)
            at += 8
            end = strings.index(0, offset)
            stack[-1][strings[offset:end].decode()] = structure[at:at + count]
            at += (count + 3) & ~3
        elif token == 9:
            break
        else:
            assert token == 4, token
    assert plic and insertion
    def align(data):
        return data + bytes((-len(data)) % 4)
    def prop(name, value):
        offset = len(strings)
        strings.extend(name.encode() + b'\0')
        return struct.pack('>III', 3, len(value), offset) + align(value)
    nodes = bytearray()
    for i, name in enumerate(('provider', 'consumer', 'runtime-provider', 'runtime-consumer')):
        address = 0x10010000 + i * 4096
        nodes += struct.pack('>I', 1) + align(f'test@{address:x}'.encode() + b'\0')
        nodes += prop('compatible', f'codex,{name}'.encode() + b'\0')
        nodes += prop('reg', struct.pack('>4I', 0, address, 0, 4096))
        nodes += prop('interrupt-parent', plic)
        nodes += prop('interrupts', struct.pack('>I', 11))
        nodes += struct.pack('>I', 2)
    structure = structure[:insertion] + nodes + structure[insertion:]
    result = bytearray(blob[:words[2]]) + structure + strings
    words[1], words[3], words[8], words[9] = len(result), words[2] + len(structure), len(strings), len(structure)
    struct.pack_into('>10I', result, 0, *words)
    return result


def build(args, directory, name, compatible, dependencies=(), flags=None, vendor=0,
          match_count=1, invalid_dependency_pointer=False):
    source = directory / (name + '.c')
    deps = ','.join('{"' + item + '"}' for item in dependencies) or '{""}'
    dependency_pointer = '(const struct xv_kmod_dependency *)1' if invalid_dependency_pointer else 'dependencies'
    source.write_text(f'''#include "kernel/io/kmodule_v3.h"
static int attach(const struct xv_kmod_host_v2 *host, const struct xv_kmod_device_v3 *device)
{{
  unsigned long satp;
  __asm__ volatile("csrr %0, satp" : "=r"(satp));
  return satp && host->abi == XV_KMOD_ABI_V2 && device->base.base.mmio ? 0 : -1;
}}
static const struct xv_kmod_match matches[] = {{ {{
  .bus = XV_KMOD_BUS_PLATFORM,
  .flags = {flags or 'XV_KMOD_MATCH_COMPATIBLE'}, .vendor = {vendor},
  .compatible = "{compatible}"
}} }};
static const struct xv_kmod_dependency dependencies[] = {{ {deps} }};
static const struct xv_kmod_driver_v3 driver = {{
  {{ sizeof(driver), XV_KMOD_ABI_V3, XV_KMOD_BUS_PLATFORM, 1, "{name}", 0 }},
  matches, {match_count}, {len(dependencies)}, {dependency_pointer}, attach
}};
__attribute__((visibility("default"))) const struct xv_kmod_driver *xv_driver_entry(unsigned abi)
{{ return abi == XV_KMOD_ABI_V3 ? &driver.base : 0; }}
''')
    binary = directory / (name + '.elf')
    subprocess.run([args.compiler, '--target=riscv64-unknown-elf', '-fuse-ld=lld', '-x', 'c',
                    '-march=rv64imac', '-mabi=lp64', '-mno-relax', '-msmall-data-limit=0',
                    '-fPIC', '-ffreestanding', '-fno-builtin', '-fvisibility=hidden', '-O2',
                    '-nostdlib', '-I', str(ROOT), str(source), '-o', str(binary),
                    '-Wl,-shared,-Bsymbolic,--no-undefined,--no-relax,-e,xv_driver_entry',
                    '-Wl,-z,max-page-size=4096,-z,separate-code'], check=True)
    return binary


def main():
    parser = argparse.ArgumentParser()
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    originals = {p: digest(p) for p in (args.kernel, args.bootstrap, args.image)}
    with tempfile.TemporaryDirectory(prefix='xv-driver-dependencies-') as temporary:
        directory = Path(temporary)
        binaries = {}
        for name, compatible, deps in (
                ('dep_consumer', 'codex,consumer', ('dep_provider',)),
                ('dep_provider', 'codex,provider', ()),
                ('rt_consumer', 'codex,runtime-consumer', ('rt_provider',)),
                ('rt_provider', 'codex,runtime-provider', ()),
                ('cycle_a', 'codex,unused', ('cycle_b',)),
                ('cycle_b', 'codex,unused', ('cycle_a',)),
                ('self_cycle', 'codex,unused', ('self_cycle',)),
                ('duplicate_dep', 'codex,unused', ('rt_provider', 'rt_provider'))):
            binaries[name] = build(args, directory, name, compatible, deps)
        binaries['bad_flags'] = build(args, directory, 'bad_flags', 'codex,unused', flags='128')
        binaries['bad_match_count'] = build(args, directory, 'bad_match_count', 'codex,unused', match_count=33)
        binaries['bad_dep_pointer'] = build(args, directory, 'bad_dep_pointer', 'codex,unused',
                                             ('rt_provider',), invalid_dependency_pointer=True)
        binaries['wrong_vendor'] = build(args, directory, 'wrong_vendor', 'codex,runtime-consumer',
                                         flags='XV_KMOD_MATCH_VENDOR | XV_KMOD_MATCH_COMPATIBLE', vendor=1)
        blob = directory / 'machine.dtb'
        subprocess.run([args.qemu, '-machine', f'virt,dumpdtb={blob}', '-bios', 'none',
                        '-m', '128M', '-smp', str(args.cpus), '-nographic'], check=True, timeout=15)
        blob.write_bytes(test_devices(blob.read_bytes()))
        base, assets = unpack(args.bootstrap.read_bytes())
        manifest = next(content for name, content, _ in assets if name == 'drivers/boot.list')
        assets = [asset for asset in assets if asset[0] != 'drivers/boot.list']
        # Consumer intentionally precedes its provider in the boot manifest.
        for name in ('dep_consumer', 'dep_provider'):
            assets.append((f'drivers/{name}.elf', binaries[name].read_bytes(), False))
            manifest += f'drivers/{name}.elf\n'.encode()
        assets.append(('drivers/boot.list', manifest, False))
        args.bootstrap = directory / 'bootstrap.elf'
        args.bootstrap.write_bytes(pack(base, assets))
        image = directory / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'driver-dependencies-{args.cpus}cpu.log',
                      network=True, extra=['-dtb', str(blob)])
        server = Server(directory)
        try:
            login_root(guest.expect, guest.process)
            output = guest.output
            assert output.index(b'driver: bound dep_provider') < output.index(b'driver: bound dep_consumer')
            listing = guest.command('drvctl --raw list')
            ids = {}
            for name in ('dep_consumer', 'dep_provider'):
                row = next(line for line in listing.splitlines() if f' {name} bound=' in line)
                assert 'bound=1' in row.split() and 'abi=3' in row.split(), row
                ids[name] = int(row.split()[0])
            guest.command('drvctl status ' + str(ids['dep_consumer']), 'State           : ready')
            def load(name, expect=None, failure=False):
                guest.command(f'curl -fsS -m 30 -o /tmp/{name}.elf {server.url}/{name}.elf', timeout=45)
                text = guest.command(f'drvctl load /tmp/{name}.elf', expect, failure=failure)
                if not failure:
                    return int(re.search(r'loaded module (\d+)', text)[1])
            wrong = load('wrong_vendor', 'automatically bound 0 devices')
            guest.command('drvctl status ' + str(wrong), 'State           : loaded')
            consumer = load('rt_consumer', 'automatically bound 0 devices')
            guest.command('drvctl status ' + str(consumer), 'State           : waiting')
            provider = load('rt_provider', 'automatically bound 1 devices')
            guest.command('drvctl status ' + str(provider), 'State           : ready')
            guest.command('drvctl status ' + str(consumer), 'State           : ready')
            devices = guest.command('drvctl --raw devices')
            for line in devices.splitlines():
                if ' test bus=' in line:
                    info = guest.command('drvctl inspect ' + line.split()[0])
                    if 'codex,runtime-consumer' not in info:
                        continue
                    assert 'Match flags     : 0x20' in info and 'Binding         : unbound' not in info
                    break
            else:
                raise AssertionError('runtime consumer identity absent')
            load('cycle_a', 'automatically bound 0 devices')
            before_rejections = guest.command('drvctl --raw list')
            before_count = re.search(r'(\d+) resident modules', before_rejections)[1]
            for name in ('cycle_b', 'self_cycle', 'duplicate_dep', 'bad_flags',
                         'bad_match_count', 'bad_dep_pointer'):
                load(name, 'drvctl: load failed', failure=True)
            assert re.search(r'(\d+) resident modules', guest.command('drvctl --raw list'))[1] == before_count
            guest.command('sync')
        finally:
            guest.close()
            server.close()
    for path, checksum in originals.items():
        assert digest(path) == checksum, path
    print('Driver dependency boot ordering, deferred binding, real ABI3 loading, '
          'vendor rejection, cycle/invalid descriptor rejection and identity inspection PASS')


if __name__ == '__main__':
    main()
