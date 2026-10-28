#!/usr/bin/env python3
"""Offline rootfs updates preserve private state and fail without partial writes."""
import argparse
import fcntl
from pathlib import Path
import shutil
import socket
import struct
import subprocess
import sys
import tempfile
import time
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from create_rootfs import BLOCK, DIRECTORY, FILE, SYMLINK, Image

p = argparse.ArgumentParser()
p.add_argument('--image', type=Path, required=True)
p.add_argument('--manifest', type=Path, required=True)
p.add_argument('--qemu', required=True)
a = p.parse_args()


def run(seed, image, manifest, *extra, success=True):
    result = subprocess.run([sys.executable, str(ROOT / 'tools/update_rootfs.py'), '--seed', str(seed),
        '--image', str(image), '--manifest', str(manifest), *extra], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    assert (result.returncode == 0) == success, result.stdout.decode()
    return result.stdout


def child(image, parent, name, content, attributes=(0, 0, 0o644), kind=FILE):
    number = image.create(parent, name, kind, attributes)
    image.append(number, content)
    return number


with tempfile.TemporaryDirectory(prefix='rootfs-update-contract-') as temporary:
    root = Path(temporary)
    seed, target = root / 'seed.img', root / 'rootfs.img'
    shutil.copyfile(a.image, seed)
    shutil.copyfile(seed, target)
    image = Image(target)
    top = image.entries(1)
    etc, home, rhome, binary = (top[name] for name in (b'etc', b'home', b'root', b'bin'))
    shadow = child(image, etc, b'shadow', b'PRIVATE_SHADOW\n', (0, 0, 0o600))
    passwd = child(image, etc, b'passwd', b'root:!:0:0:root:/root:/bin/sh\n')
    group = child(image, etc, b'group', b'root:!:0:\n')
    counter = child(image, etc, b'idnext', b'1042\n', (0, 0, 0o600))
    alice = image.create(home, b'alice', DIRECTORY, (1000, 1001, 0o750))
    note = child(image, alice, b'note', b'USER_CONTENT\n', (1000, 1001, 0o640))
    private = child(image, rhome, b'keep', b'ROOT_CONTENT\n', (0, 0, 0o600))
    image.unlink_file(binary, b'echo')
    echo = child(image, binary, b'echo', b'S' * 20480, (1000, 1001, 0o777))
    image.link(rhome, b'echo-alias', echo)
    image.unlink_file(binary, b'pwd')
    child(image, binary, b'pwd', b'/etc/shadow', (0, 0, 0o777), SYMLINK)
    image.set_attributes(image.entries(binary)[b'passwd'], (1000, 1001, 0o700))
    image.set_attributes(binary, (1000, 1001, 0o777))
    extra = child(image, binary, b'extra', b'UNLISTED_PROGRAM\n', (1000, 1001, 0o750))
    tests = image.entries(top[b'usr'])[b'tests']
    image.unlink_file(tests, b'vmcheck')
    image.validate_allocations()
    target.write_bytes(image.data)
    original = target.read_bytes()
    seed_original = seed.read_bytes()
    dry = run(seed, target, a.manifest, '--dry-run')
    assert b'dry run:' in dry and target.read_bytes() == original
    assert not list(root.glob('*.before-update-*.img'))

    # Resource exhaustion must not publish any of the preceding prepared files.
    for full in ('inodes', 'blocks'):
        candidate = root / (full + '.img')
        exhausted = Image.from_bytes(original)
        if full == 'inodes':
            for inode in range(2, exhausted.inodes):
                if not exhausted.kind(inode):
                    struct.pack_into('<H', exhausted.data, exhausted.inode(inode), FILE)
        else:
            for block in range(exhausted.bitmap + exhausted.blocks // 8192 + 1, exhausted.blocks):
                exhausted.data[exhausted.bitmap * BLOCK + block // 8] |= 1 << (block % 8)
        candidate.write_bytes(exhausted.data)
        before = candidate.read_bytes()
        run(seed, candidate, a.manifest, success=False)
        assert candidate.read_bytes() == before and not list(root.glob(candidate.name + '.before-update-*.img'))

    bad_manifest = root / 'bad.tsv'
    bad_manifest.write_text(a.manifest.read_text() + 'file\t0600\t/etc/shadow\tignored\n')
    run(seed, target, bad_manifest, success=False)
    assert target.read_bytes() == original
    bad = root / 'bad-parent.img'
    malformed = Image.from_bytes(original)
    struct.pack_into('<H', malformed.data, malformed.inode(binary), SYMLINK)
    bad.write_bytes(malformed.data)
    before = bad.read_bytes()
    run(seed, bad, a.manifest, success=False)
    assert bad.read_bytes() == before

    # Reject both POSIX writers and the actual QEMU OFD permission locks.
    locker = subprocess.Popen([sys.executable, '-c',
        'import fcntl,sys; f=open(sys.argv[1],"r+b"); fcntl.lockf(f,fcntl.LOCK_EX); print("ready",flush=True); sys.stdin.read()',
        str(target)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        assert locker.stdout.readline() == b'ready\n'
        assert b'image is in use' in run(seed, target, a.manifest, success=False)
    finally:
        locker.communicate()
    qmp = root / 'qmp.sock'
    guest = subprocess.Popen([a.qemu, '-machine', 'virt', '-bios', 'none', '-S', '-display', 'none',
        '-serial', 'none', '-monitor', 'none', '-qmp', f'unix:{qmp},server=on,wait=off',
        '-drive', f'file={target},if=none,format=raw,id=x0', '-device', 'virtio-blk-device,drive=x0'], stderr=subprocess.DEVNULL)
    try:
        deadline = time.monotonic() + 5
        while not qmp.exists():
            assert time.monotonic() < deadline and guest.poll() is None
            time.sleep(.01)
        with socket.socket(socket.AF_UNIX) as connection:
            connection.settimeout(5)
            connection.connect(str(qmp))
            with connection.makefile('rwb') as channel:
                channel.readline()
                channel.write(b'{"execute":"qmp_capabilities"}\n')
                channel.flush()
                channel.readline()
                assert b'image is in use' in run(seed, target, a.manifest, success=False)
    finally:
        guest.terminate()
        guest.wait(timeout=5)
    assert target.read_bytes() == original

    # A failure at the publication boundary leaves the original and a complete backup.
    import update_rootfs
    fault = root / 'publication-failure.img'
    fault.write_bytes(original)
    with mock.patch.object(update_rootfs.os, 'replace', side_effect=OSError('injected publication failure')):
        try:
            update_rootfs.update(seed, fault, a.manifest)
        except OSError:
            pass
        else:
            raise AssertionError('publication error was not propagated')
    assert fault.read_bytes() == original
    saved = list(root.glob(fault.name + '.before-update-*.img'))
    assert len(saved) == 1 and saved[0].read_bytes() == original

    output = run(seed, target, a.manifest)
    assert b'rootfs updated:' in output
    backups = list(root.glob(target.name + '.before-update-*.img'))
    assert len(backups) == 1 and backups[0].read_bytes() == original
    assert backups[0].stat().st_mode & 0o777 == 0o600
    updated = Image(target)
    for inode in (shadow, passwd, group, counter, note, private, extra):
        assert updated.read(inode) == image.read(inode)
        assert updated.attributes(inode) == image.attributes(inode)
    assert updated.read(echo) == b'S' * 20480 and updated.attributes(echo) == (1000, 1001, 0o777)
    assert updated.entries(rhome)[b'echo-alias'] == echo
    fresh = Image(seed)
    for line in a.manifest.read_text().splitlines():
        kind, mode, name, *unused = line.split('\t')
        if kind != 'file': continue
        def find(volume):
            node = 1
            for part in name.split('/')[1:]: node = volume.entries(node)[part.encode()]
            return node
        installed, source = find(updated), find(fresh)
        assert updated.kind(installed) == FILE and updated.attributes(installed) == (0, 0, int(mode, 8))
        assert updated.read(installed) == fresh.read(source)
    assert updated.attributes(binary) == (0, 0, 0o755)
    assert updated.kind(updated.entries(binary)[b'pwd']) == FILE
    stable = target.read_bytes()
    assert b'already up to date' in run(seed, target, a.manifest)
    assert target.read_bytes() == stable and len(list(root.glob(target.name + '.before-update-*.img'))) == 1
    assert seed.read_bytes() == seed_original
    # Repeated replacement reuses directory holes instead of growing forever.
    directory_size = len(updated.read(binary))
    for iteration in range(2):
        current = Image(target)
        program = current.entries(binary)[b'echo']
        current.set_attributes(program, (0, 0, 0o700))
        target.write_bytes(current.data)
        run(seed, target, a.manifest)
        assert len(Image(target).read(binary)) == directory_size

    # Journal replay is applied in the replacement while the backup remains exact.
    journal = root / 'journal.img'
    logged = Image.from_bytes(original)
    block = logged.block(private, 0)
    replacement = b'LOGGED_DATA!\n'
    assert len(replacement) == len(logged.read(private))
    struct.pack_into('<II', logged.data, logged.log * BLOCK, 1, block)
    logged.data[(logged.log + 1) * BLOCK:(logged.log + 2) * BLOCK] = replacement + bytes(BLOCK - len(replacement))
    journal.write_bytes(logged.data)
    pending = journal.read_bytes()
    run(seed, journal, a.manifest)
    assert Image(journal).read(private) == replacement
    assert list(root.glob(journal.name + '.before-update-*.img'))[0].read_bytes() == pending

print('Rootfs update preservation, atomic failure, lock, backup and idempotence contracts passed')
