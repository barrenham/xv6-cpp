#!/usr/bin/env python3
"""Validate installed image paths, metadata, contents and manifest failures."""
import argparse
from pathlib import Path
import struct
import subprocess
import tempfile


class Image:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        self.super = struct.unpack_from('<8I', self.data, 1024)
        assert self.super[0] == 0x10203041

    def inode(self, number):
        offset = self.super[6] * 1024 + number * 128
        kind, major, minor, links, size = struct.unpack_from('<hhhhI', self.data, offset)
        blocks = struct.unpack_from('<13I', self.data, offset + 12)
        uid, gid, mode = struct.unpack_from('<IIH', self.data, offset + 64)
        return dict(kind=kind, links=links, size=size, blocks=blocks, uid=uid, gid=gid, mode=mode)

    def contents(self, node):
        addresses = list(node['blocks'][:12])
        if node['blocks'][12]:
            addresses += list(struct.unpack_from('<256I', self.data, node['blocks'][12] * 1024))
        output = bytearray()
        for address in addresses[:(node['size'] + 1023) // 1024]:
            assert address, 'missing data block'
            output += self.data[address * 1024:(address + 1) * 1024]
        return bytes(output[:node['size']])

    def entries(self, number):
        raw = self.contents(self.inode(number))
        return {name.split(b'\0', 1)[0].decode(): child
                for child, name in struct.iter_unpack('<H14s', raw) if child}

    def lookup(self, path):
        number = 1
        for component in path.split('/')[1:]:
            number = self.entries(number)[component]
        return number, self.inode(number)


def verify(image, manifest):
    disk = Image(image)
    paths = []
    for line in Path(manifest).read_text().splitlines():
        if not line or line.startswith('#'):
            continue
        kind, mode, path, *source = line.split('\t')
        number, node = disk.lookup(path)
        assert node['kind'] == (1 if kind == 'dir' else 2), path
        assert node['mode'] == int(mode, 8) and node['uid'] == node['gid'] == 0, path
        if source:
            assert disk.contents(node) == Path(source[0]).read_bytes(), path
        else:
            entries = disk.entries(number)
            assert entries['.'] == number
            parent = path.rsplit('/', 1)[0]
            parent_number = disk.lookup(parent)[0] if parent else 1
            assert entries['..'] == parent_number, path
            children = sum(disk.inode(child)['kind'] == 1
                           for name, child in entries.items() if name not in ('.', '..'))
            assert node['links'] == 1 + children, path
        paths.append(path)
    expected_root = {path.split('/')[1] for path in paths}
    assert set(disk.entries(1)) - {'.', '..'} == expected_root
    assert all(disk.inode(child)['kind'] == 1 for name, child in disk.entries(1).items()
               if name not in ('.', '..')), 'root contains ordinary files'
    return disk


def contracts(mkfs):
    with tempfile.TemporaryDirectory(prefix='xv image layout ') as directory:
        root = Path(directory)
        source = root / 'input data.bin'
        source.write_bytes(bytes(range(256)) * 80)
        manifest, image = root / 'manifest.tsv', root / 'image.img'
        text = 'dir\t0755\t/bin\ndir\t0755\t/usr\ndir\t0755\t/usr/tests\n'
        text += f'file\t0644\t/bin/data\t{source}\nfile\t0755\t/bin/abcdefghijklmn\t{source}\n'
        text += ''.join(f'file\t0755\t/usr/tests/f{i}\t{source}\n' for i in range(80))
        manifest.write_text(text)
        subprocess.run([mkfs, str(image), '--manifest', str(manifest), '--filesystem', 'xv6'], check=True)
        verify(image, manifest)
        original = image.read_bytes()
        invalid = [
            'dir\t0755\trelative\n', 'dir\t0755\t/../escape\n',
            'dir\t0755\t/double//slash\n', 'dir\t0755\t/name-too-long-123\n',
            'dir\t4755\t/bin\n', 'dir\t0755\t/bin\ndir\t0755\t/bin\n',
            f'file\t0755\t/missing/parent\t{source}\n',
            f'file\t0755\t/file\t{source}\ndir\t0755\t/file/child\n',
            f'file\t0644\t/absent\t{root / "missing"}\n',
        ]
        too_big = root / 'too-big.bin'
        too_big.write_bytes(b'x' * (269 * 1024))
        invalid.append(f'file\t0644\t/too-big\t{too_big}\n')
        invalid.append(''.join(f'dir\t0755\t/d{i}\n' for i in range(201)))
        for text in invalid:
            manifest.write_text(text)
            result = subprocess.run([mkfs, str(image), '--manifest', str(manifest), '--filesystem', 'xv6'], capture_output=True)
            assert result.returncode == 1 and b'mkfs:' in result.stderr, (text, result.stderr.decode())
            assert b'runtime error:' not in result.stderr and b'AddressSanitizer' not in result.stderr, result.stderr.decode()
            assert image.read_bytes() == original, 'failed build replaced the published image'
            assert not list(root.glob('*.tmp.*')), 'failed build leaked a temporary image'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mkfs', required=True)
    parser.add_argument('--image', required=True)
    parser.add_argument('--manifest', required=True)
    args = parser.parse_args()
    verify(args.image, args.manifest)
    contracts(args.mkfs)
    print('image layout: manifest, directories, modes, payloads and rollback passed')


if __name__ == '__main__':
    main()
