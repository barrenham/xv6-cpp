#!/usr/bin/env python3
import argparse
import hashlib
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from ext4_fixtures import make_image, command


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--contract', required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-ext4-native-') as temporary:
        root = Path(temporary)
        for block, checksum, seed, bits64 in ((1024, True, False, True), (2048, True, False, True),
                (4096, True, False, True), (1024, False, False, False), (4096, True, True, True)):
            directory = root / f'{block}-{checksum}-{seed}-{bits64}'; directory.mkdir()
            image = make_image(directory, block, checksums=checksum, seed=seed, bits64=bits64)
            before = hashlib.sha256(image.read_bytes()).digest()
            subprocess.run([args.contract, str(image)], check=True)
            assert hashlib.sha256(image.read_bytes()).digest() == before
            if block == 1024 and not checksum:
                data = bytearray(image.read_bytes())
                table = struct.unpack_from('<I', data, 2 * block + 8)[0]
                inode_size = struct.unpack_from('<H', data, 1024 + 88)[0]
                first = struct.unpack_from('<I', data, table * block + inode_size + 60)[0]
                struct.pack_into('<H', data, first * block + 4, 0)
                damaged = directory / 'bad-record.img'; damaged.write_bytes(data)
                subprocess.run([args.contract, str(damaged), '--bad-directory'], check=True)
            if block != 1024 or not checksum or seed:
                continue
            for label, edit, mode in (
                    ('dirty', 'feature needs_recovery', '--reject'),
                    ('inline', 'feature inline_data', '--reject'),
                    ('bad-geometry', 'set_super_value blocks_per_group 0', '--reject'),
                    ('bad-depth', 'set_inode_field /fragmented block[1] 393220', '--bad-file'),
                    ('bad-child', 'set_inode_field /fragmented block[4] 4294967295', '--bad-file')):
                changed = directory / (label + '.img'); shutil.copyfile(image, changed)
                command([shutil.which('debugfs') or '/usr/sbin/debugfs', '-w', '-R', edit, str(changed)])
                subprocess.run([args.contract, str(changed), mode], check=True)
            data = image.read_bytes()
            superblock = data[1024:2048]
            inode_size = struct.unpack_from('<H', superblock, 88)[0]
            table = struct.unpack_from('<I', data, 2 * block + 8)[0]
            root_inode = table * block + inode_size
            inode_bitmap = struct.unpack_from('<I', data, 2 * block + 4)[0]
            root_data = struct.unpack_from('<I', data, root_inode + 60)[0]
            mutations = {'bad-super': 1024 + 300, 'bad-group': 2 * block + 8,
                         'bad-inode': root_inode + 8, 'bad-bitmap': inode_bitmap * block,
                         'bad-directory': root_data * block + 8,
                         'needs-recovery': 1024 + 96, 'bad-size': 1024 + 24}
            for name, at in mutations.items():
                copy = bytearray(data)
                copy[at] ^= 4 if name == 'needs-recovery' else 0x80
                target = directory / (name + '.img'); target.write_bytes(copy)
                subprocess.run([args.contract, str(target), '--reject'], check=True)
            stat = command([shutil.which('debugfs') or '/usr/sbin/debugfs', '-R', 'stat /fragmented', str(image)]).stdout.decode()
            import re
            extent = re.search(r'\(ETB0\):(\d+)', stat)
            assert extent, stat
            damaged = bytearray(data); damaged[int(extent[1]) * block + 20] ^= 0x40
            target = directory / 'bad-extent.img'; target.write_bytes(damaged)
            subprocess.run([args.contract, str(target), '--bad-file'], check=True)
        print('ext4: five independent layouts, corruption rejection and zero writes passed')


if __name__ == '__main__':
    main()
