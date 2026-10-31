#!/usr/bin/env python3
"""Offline installation of manifest-owned system files into a persistent rootfs.

The live image is locked throughout planning and publication. Changes are made
in a private copy, an exact backup is published first, then one atomic rename
installs the complete update. Accounts and user directories are never selected.
"""
import argparse
from datetime import datetime, timezone
import fcntl
import os
from pathlib import Path, PurePosixPath
import stat
import struct
import tempfile
import uuid

from create_rootfs import BLOCK, DIRECTORY, FILE, SYMLINK, Image, clean_seed


def manifest_files(path, name_max=255):
    files, directories, names = [], {}, set()
    for line in path.read_text().splitlines():
        if not line or line.startswith('#'):
            continue
        fields = line.split('\t')
        if len(fields) < 3 or fields[0] not in ('dir', 'file'):
            raise ValueError('invalid installation manifest')
        kind, mode, name = fields[:3]
        if len(fields) != (3 if kind == 'dir' else 4):
            raise ValueError('invalid installation manifest field count')
        guest = PurePosixPath(name)
        parts = name.split('/')[1:]
        if (not name.startswith('/') or not parts or len(name.encode()) >= 4096 or
                any(not part or '\0' in part or part in ('.', '..') or len(part.encode()) > name_max for part in parts) or
                str(guest) != name or name in names):
            raise ValueError('invalid or duplicate installation path')
        if not mode or len(mode) > 5 or any(digit not in '01234567' for digit in mode):
            raise ValueError('invalid installation mode')
        mode = int(mode, 8)
        if mode > 0o7777 or (kind == 'dir' and mode & 0o6000):
            raise ValueError('unsupported installation mode')
        names.add(name)
        if kind == 'dir':
            directories[name] = mode
        else:
            if not name.startswith(('/bin/', '/sbin/', '/usr/tests/', '/usr/share/doc/')):
                raise ValueError('manifest attempts to update a protected path: ' + name)
            files.append((guest, mode))
    if not files:
        raise ValueError('installation manifest has no system files')
    return files, directories


def lookup(image, parts):
    number = image.root
    for part in parts:
        entry = image.entries(number).get(part.encode())
        if entry is None:
            raise ValueError('installation seed is missing a manifest path')
        number = entry
    return number


def install(image, seed, files, directories):
    changed = []
    directory_changes = set()
    for path, mode in files:
        source = lookup(seed, path.parts[1:])
        if seed.kind(source) != FILE or seed.attributes(source) != (0, 0, mode):
            raise ValueError('seed type/ownership/mode differs from manifest: ' + str(path))
        content = seed.read(source)
        parent = image.root
        for index, part in enumerate(path.parts[1:-1], 2):
            name = part.encode()
            entry = image.entries(parent).get(name)
            full = str(PurePosixPath(*path.parts[:index]))
            permissions = directories.get(full, 0o755)
            if entry is None:
                entry = image.create(parent, name, DIRECTORY, (0, 0, permissions))
                directory_changes.add(full)
            elif image.kind(entry) != DIRECTORY:
                raise ValueError('system directory is not a real directory: ' + full)
            if image.attributes(entry) != (0, 0, permissions):
                image.set_attributes(entry, (0, 0, permissions))
                directory_changes.add(full)
            parent = entry
        name = path.name.encode()
        existing = image.entries(parent).get(name)
        if existing is not None:
            if image.kind(existing) == FILE and image.attributes(existing) == (0, 0, mode) and image.read(existing) == content:
                continue
            if image.kind(existing) not in (FILE, SYMLINK):
                raise ValueError('system file conflicts with a directory/device: ' + str(path))
            # Replace this name only. Hard-linked user files and symlink targets
            # keep their old contents and ownership.
            image.unlink_file(parent, name)
        target = image.create(parent, name, FILE, (0, 0, mode))
        image.append(target, content)
        changed.append(str(path))
    image.validate_allocations()
    return changed, sorted(directory_changes)


def fsync_directory(path):
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def same_inode(path, descriptor):
    named, opened = path.stat(), os.fstat(descriptor)
    return (named.st_dev, named.st_ino) == (opened.st_dev, opened.st_ino)


def update(seed_path, image_path, manifest, dry_run=False):
    seed = Image(seed_path)
    files, directories = manifest_files(manifest, seed.name_max)
    clean_seed(seed)
    seed.validate_allocations()
    path = image_path.resolve(strict=True)
    if path.samefile(seed_path):
        raise ValueError('rootfs and installation seed must be different files')
    # Never reopen this inode while holding its POSIX lock: closing another
    # descriptor for the same file would release this process's record locks.
    with path.open('r+b') as original:
        if not stat.S_ISREG(os.fstat(original.fileno()).st_mode):
            raise ValueError('rootfs is not a regular image file')
        fcntl.lockf(original, fcntl.LOCK_EX | fcntl.LOCK_NB)
        if not same_inode(path, original.fileno()):
            raise ValueError('rootfs path changed while acquiring its lock; retry')
        content = original.read()
        image = Image.from_bytes(content)
        image.validate_allocations()
        changed, folders = install(image, seed, files, directories)
        for name in changed:
            print(('would install: ' if dry_run else 'prepared: ') + name)
        if not changed and not folders:
            print('rootfs already up to date')
            return
        if dry_run:
            print(f'dry run: {len(changed)} files, {len(folders)} system directories; image unchanged')
            return
        stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
        backup = path.with_name(path.name + '.before-update-' + stamp + '-' + uuid.uuid4().hex[:8] + '.img')
        with tempfile.TemporaryDirectory(prefix='.rootfs-update-', dir=path.parent) as temporary:
            temporary = Path(temporary)
            candidate = temporary / 'updated.img'
            with candidate.open('x+b') as output:
                os.chmod(candidate, 0o600)
                fcntl.lockf(output, fcntl.LOCK_EX | fcntl.LOCK_NB)
                output.write(image.data)
                output.flush()
                os.fsync(output.fileno())
                before = temporary / 'before.img'
                with before.open('xb') as saved:
                    os.chmod(before, 0o600)
                    saved.write(content)
                    saved.flush()
                    os.fsync(saved.fileno())
                if not same_inode(path, original.fileno()):
                    raise ValueError('rootfs path changed during update; retry')
                os.link(before, backup)
                fsync_directory(path.parent)
                os.replace(candidate, path)
                fsync_directory(path.parent)
        print(f'rootfs updated: {len(changed)} files, {len(folders)} system directories')
        print('backup:', backup)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--seed', required=True, type=Path)
    parser.add_argument('--image', required=True, type=Path)
    parser.add_argument('--manifest', required=True, type=Path)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    update(args.seed, args.image, args.manifest, args.dry_run)


if __name__ == '__main__':
    try:
        main()
    except BlockingIOError:
        raise SystemExit('rootfs-update: image is in use; exit QEMU before updating')
    except (OSError, ValueError, struct.error) as error:
        raise SystemExit('rootfs-update: ' + str(error))
