#!/usr/bin/env python3
"""Publish a persistent xv6 rootfs once, optionally importing an old data volume.

Existing rootfs and import images are never modified. All edits happen in a
private seed copy before atomic, no-replace publication.
"""
import argparse
import fcntl
import os
from pathlib import Path
import struct
import tempfile

BLOCK = 1024
INODE = 128
DIRECT = 12
DIRECTORY, FILE, DEVICE, SYMLINK = 1, 2, 3, 5


class Image:
    def __init__(self, path):
        with Path(path).open('r+b') as source:
            # No content writes: O_RDWR is needed only for an exclusive
            # record lock, which conflicts with QEMU's active OFD locks.
            fcntl.lockf(source, fcntl.LOCK_EX | fcntl.LOCK_NB)
            self.data = bytearray(source.read())
        self._decode()

    @classmethod
    def from_bytes(cls, content):
        image = cls.__new__(cls)
        image.data = bytearray(content)
        image._decode()
        return image

    def _decode(self):
        if len(self.data) < 2 * BLOCK:
            raise ValueError('image is too short')
        (magic, self.blocks, _, self.inodes, self.log_blocks,
         self.log, self.inode_start, self.bitmap) = struct.unpack_from('<8I', self.data, BLOCK)
        if (magic != 0x10203041 or self.blocks * BLOCK != len(self.data) or
                not 2 <= self.log < self.inode_start < self.bitmap < self.blocks or
                self.inode_start + (self.inodes + 7) // 8 > self.bitmap):
            raise ValueError('unsupported or invalid xv6 image layout')
        # Apply committed journal contents to this private in-memory view only.
        count = struct.unpack_from('<I', self.data, self.log * BLOCK)[0]
        if count >= self.log_blocks or count > (BLOCK - 4) // 4:
            raise ValueError('invalid journal header')
        for index in range(count):
            target = struct.unpack_from('<I', self.data, self.log * BLOCK + 4 + index * 4)[0]
            if not self.bitmap <= target < self.blocks and not self.inode_start <= target < self.bitmap:
                raise ValueError('invalid journal destination')
            source = (self.log + 1 + index) * BLOCK
            self.data[target * BLOCK:(target + 1) * BLOCK] = self.data[source:source + BLOCK]
        self.data[self.log * BLOCK:(self.log + 1) * BLOCK] = bytes(BLOCK)

    def inode(self, number):
        if not 0 < number < self.inodes:
            raise ValueError('invalid inode number')
        return self.inode_start * BLOCK + number * INODE

    def kind(self, number):
        return struct.unpack_from('<H', self.data, self.inode(number))[0]

    def attributes(self, number):
        return struct.unpack_from('<IIH', self.data, self.inode(number) + 64)

    def set_attributes(self, number, attributes):
        struct.pack_into('<IIH', self.data, self.inode(number) + 64, *attributes)

    def allocate_block(self):
        first = self.bitmap + self.blocks // (BLOCK * 8) + 1
        for number in range(first, self.blocks):
            offset = self.bitmap * BLOCK + number // 8
            bit = 1 << (number % 8)
            if not self.data[offset] & bit:
                self.data[offset] |= bit
                self.data[number * BLOCK:(number + 1) * BLOCK] = bytes(BLOCK)
                return number
        raise ValueError('rootfs has insufficient free blocks for import')

    def block(self, number, index, create=False):
        if index >= DIRECT + BLOCK // 4:
            raise ValueError('file exceeds xv6 maximum size')
        offset = self.inode(number) + 12 + min(index, DIRECT) * 4
        address = struct.unpack_from('<I', self.data, offset)[0]
        if index >= DIRECT:
            if not address and create:
                address = self.allocate_block()
                struct.pack_into('<I', self.data, offset, address)
            if not address:
                return 0
            if not self.bitmap + self.blocks // (BLOCK * 8) + 1 <= address < self.blocks:
                raise ValueError('invalid indirect block')
            offset = address * BLOCK + (index - DIRECT) * 4
            address = struct.unpack_from('<I', self.data, offset)[0]
        if not address and create:
            address = self.allocate_block()
            struct.pack_into('<I', self.data, offset, address)
        if address and not self.bitmap + self.blocks // (BLOCK * 8) + 1 <= address < self.blocks:
            raise ValueError('invalid data block')
        return address

    def read(self, number):
        size = struct.unpack_from('<I', self.data, self.inode(number) + 8)[0]
        if size > (DIRECT + BLOCK // 4) * BLOCK:
            raise ValueError('invalid inode size')
        result = bytearray()
        for index in range((size + BLOCK - 1) // BLOCK):
            block = self.block(number, index)
            result.extend(self.data[block * BLOCK:(block + 1) * BLOCK] if block else bytes(BLOCK))
        return bytes(result[:size])

    def append(self, number, content):
        offset = struct.unpack_from('<I', self.data, self.inode(number) + 8)[0]
        size = offset + len(content)
        while content:
            address = self.block(number, offset // BLOCK, create=True)
            count = min(len(content), BLOCK - offset % BLOCK)
            start = address * BLOCK + offset % BLOCK
            self.data[start:start + count] = content[:count]
            content = content[count:]
            offset += count
        struct.pack_into('<I', self.data, self.inode(number) + 8, size)

    def write_at(self, number, offset, content):
        size = struct.unpack_from('<I', self.data, self.inode(number) + 8)[0]
        if offset < 0 or offset + len(content) > size:
            raise ValueError('write outside existing inode size')
        while content:
            block = self.block(number, offset // BLOCK)
            if not block:
                raise ValueError('unexpected hole in directory')
            count = min(len(content), BLOCK - offset % BLOCK)
            start = block * BLOCK + offset % BLOCK
            self.data[start:start + count] = content[:count]
            content = content[count:]
            offset += count

    def owned_blocks(self, number):
        offset = self.inode(number)
        addresses = struct.unpack_from('<13I', self.data, offset + 12)
        result = [block for block in addresses[:DIRECT] if block]
        if addresses[DIRECT]:
            indirect = addresses[DIRECT]
            if not self.bitmap + self.blocks // (BLOCK * 8) + 1 <= indirect < self.blocks:
                raise ValueError('invalid indirect block')
            result.append(indirect)
            result.extend(block for block in struct.unpack_from('<256I', self.data, indirect * BLOCK) if block)
        return result

    def validate_allocations(self):
        owned = set()
        first = self.bitmap + self.blocks // (BLOCK * 8) + 1
        if self.kind(1) != DIRECTORY:
            raise ValueError('invalid root directory')
        for number in range(1, self.inodes):
            kind = self.kind(number)
            if not kind:
                continue
            if kind not in (DIRECTORY, FILE, DEVICE, SYMLINK):
                raise ValueError('unsupported inode type')
            size = struct.unpack_from('<I', self.data, self.inode(number) + 8)[0]
            if size > (DIRECT + BLOCK // 4) * BLOCK or (kind == DIRECTORY and size % 16):
                raise ValueError('invalid inode size')
            for block in self.owned_blocks(number):
                if not first <= block < self.blocks or block in owned:
                    raise ValueError('invalid or multiply owned disk block')
                if not self.data[self.bitmap * BLOCK + block // 8] & (1 << (block % 8)):
                    raise ValueError('live inode references a free disk block')
                owned.add(block)

    def unlink_file(self, parent, name):
        content = self.read(parent)
        for offset in range(0, len(content), 16):
            number, entry_name = struct.unpack_from('<H14s', content, offset)
            if not number or entry_name.rstrip(b'\0') != name:
                continue
            if self.kind(number) not in (FILE, SYMLINK):
                raise ValueError('system program target is not a regular file or symlink')
            inode = self.inode(number)
            links = struct.unpack_from('<H', self.data, inode + 6)[0]
            if not links:
                raise ValueError('directory entry references an unlinked inode')
            blocks = self.owned_blocks(number) if links == 1 else []
            self.write_at(parent, offset, bytes(16))
            if links > 1:
                struct.pack_into('<H', self.data, inode + 6, links - 1)
            else:
                for block in blocks:
                    self.data[self.bitmap * BLOCK + block // 8] &= ~(1 << (block % 8))
                    self.data[block * BLOCK:(block + 1) * BLOCK] = bytes(BLOCK)
                self.data[inode:inode + INODE] = bytes(INODE)
            return
        raise ValueError('directory entry not found')

    def entries(self, number):
        if self.kind(number) != DIRECTORY:
            raise ValueError('expected a directory')
        content = self.read(number)
        if len(content) % 16:
            raise ValueError('invalid directory size')
        result = {}
        for inode, name in struct.iter_unpack('<H14s', content):
            if not inode:
                continue
            name = name.rstrip(b'\0')
            if not name or b'/' in name or b'\0' in name or name in result:
                raise ValueError('invalid or duplicate directory entry')
            self.inode(inode)
            result[name] = inode
        return result

    def link(self, parent, name, number):
        if not 0 < len(name) <= 14 or name in self.entries(parent):
            raise ValueError('invalid or conflicting import name')
        record = struct.pack('<H14s', number, name)
        directory = self.read(parent)
        for slot in range(0, len(directory), 16):
            if not struct.unpack_from('<H', directory, slot)[0]:
                self.write_at(parent, slot, record)
                break
        else:
            self.append(parent, record)
        offset = self.inode(number) + 6
        links = struct.unpack_from('<H', self.data, offset)[0]
        if links >= 32767:
            raise ValueError('too many links')
        struct.pack_into('<H', self.data, offset, links + 1)

    def create(self, parent, name, kind, attributes=(0, 0, 0o644)):
        for number in range(2, self.inodes):
            if not self.kind(number):
                break
        else:
            raise ValueError('rootfs has insufficient free inodes for import')
        offset = self.inode(number)
        self.data[offset:offset + INODE] = bytes(INODE)
        struct.pack_into('<H', self.data, offset, kind)
        self.set_attributes(number, attributes)
        self.link(parent, name, number)
        if kind == DIRECTORY:
            self.append(number, struct.pack('<H14sH14s', number, b'.', parent, b'..'))
            offset = self.inode(parent) + 6
            links = struct.unpack_from('<H', self.data, offset)[0]
            if links >= 32767:
                raise ValueError('too many directory links')
            struct.pack_into('<H', self.data, offset, links + 1)
        return number

    def import_volume(self, source):
        copied, directories = {}, set()

        def visit(old, new, prefix):
            if old in directories:
                raise ValueError('directory cycle in import volume')
            directories.add(old)
            for name, number in source.entries(old).items():
                if name in (b'.', b'..'):
                    continue
                path = prefix + b'/' + name
                kind = source.kind(number)
                attributes = source.attributes(number)
                existing = self.entries(new).get(name)
                if kind == DIRECTORY:
                    if existing is not None and self.kind(existing) != DIRECTORY:
                        raise ValueError('import conflicts with rootfs: ' + repr(path))
                    child = existing if existing is not None else self.create(new, name, kind, attributes)
                    self.set_attributes(child, attributes)
                    visit(number, child, path)
                    continue
                if kind not in (FILE, DEVICE, SYMLINK) or existing is not None:
                    raise ValueError('unsupported or conflicting import: ' + repr(path))
                if number in copied:
                    self.link(new, name, copied[number])
                    continue
                child = self.create(new, name, kind, attributes)
                copied[number] = child
                if kind == DEVICE:
                    self.data[self.inode(child) + 2:self.inode(child) + 6] = source.data[source.inode(number) + 2:source.inode(number) + 6]
                content = source.read(number)
                if path == b'/etc/passwd':
                    lines = []
                    for line in content.splitlines(keepends=True):
                        fields = line.split(b':')
                        if len(fields) != 7:
                            raise ValueError('invalid imported passwd record')
                        if fields[5] in (b'/data/root', b'/data/home') or fields[5].startswith(b'/data/home/'):
                            fields[5] = fields[5][5:]
                        lines.append(b':'.join(fields))
                    content = b''.join(lines)
                self.append(child, content)

        visit(1, 1, b'')


def clean_seed(image):
    root = image.entries(1)
    for directory in (b'root', b'home', b'data'):
        if directory in root and set(image.entries(root[directory])) - {b'.', b'..'}:
            raise ValueError('installation seed contains private home/data files')
    if b'etc' in root:
        accounts = {b'passwd', b'shadow', b'group', b'idnext', b'.users.lock'}
        if accounts.intersection(image.entries(root[b'etc'])):
            raise ValueError('installation seed contains existing account state')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--seed', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    source = parser.add_mutually_exclusive_group()
    source.add_argument('--import-data', type=Path)
    source.add_argument('--fresh', action='store_true', help='require a new, clean rootfs; fail if the output exists')
    args = parser.parse_args()
    if args.output.exists():
        if args.fresh:
            raise ValueError('fresh rootfs output already exists; choose another path')
        print('rootfs retained:', args.output)
        return
    image = Image(args.seed)
    # Validate the seed contains the essential init image before publishing.
    binary = image.entries(1).get(b'sbin')
    if binary is None or b'init' not in image.entries(binary):
        raise ValueError('seed does not contain /sbin/init')
    clean_seed(image)
    image.validate_allocations()
    if args.import_data and args.import_data.exists():
        image.import_volume(Image(args.import_data))
        print('imported existing data volume:', args.import_data)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.rootfs-', dir=args.output.parent) as temporary:
        candidate = Path(temporary) / 'rootfs.img'
        with candidate.open('xb') as output:
            os.chmod(candidate, 0o600)
            output.write(image.data)
            output.flush()
            os.fsync(output.fileno())
        try:
            os.link(candidate, args.output)
        except FileExistsError:
            if args.fresh or not args.output.exists():
                raise
        directory = os.open(args.output.parent, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(directory)
        finally:
            os.close(directory)
    print('rootfs ready:', args.output)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, struct.error) as error:
        raise SystemExit('rootfs: ' + str(error))
