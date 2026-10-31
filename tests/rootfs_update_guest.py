#!/usr/bin/env python3
"""Boot upgraded and independently created root filesystems in real QEMU."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from create_rootfs import Image, FILE

parser = argparse.ArgumentParser()
parser.add_argument('--qemu', required=True)
parser.add_argument('--kernel', required=True)
parser.add_argument('--image', type=Path, required=True)
parser.add_argument('--manifest', type=Path, required=True)
parser.add_argument('--cpus', type=int, default=5)
a = parser.parse_args()
PASSWORD = 'test-rootfs-update-owner-42'
USERPASS = 'test-rootfs-update-alice-42'
OTHERPASS = 'test-rootfs-independent-owner-43'


class Guest:
    def __init__(self, image, label):
        self.label = label
        self.text = bytearray()
        self.cursor = 0
        self.p = subprocess.Popen([a.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', a.kernel,
            '-m', '128M', '-smp', str(a.cpus), '-nographic', '-global', 'virtio-mmio.force-legacy=false',
            '-drive', f'file={image},if=none,format=raw,id=x0', '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
        self.selector = selectors.DefaultSelector()
        self.selector.register(self.p.stdout, selectors.EVENT_READ)

    def send(self, content):
        self.p.stdin.write(content if isinstance(content, bytes) else (content + '\n').encode())
        self.p.stdin.flush()

    def expect(self, token):
        token = token.encode()
        deadline = time.monotonic() + 45
        while True:
            found = self.text.find(token, self.cursor)
            if found >= 0:
                self.cursor = found + len(token)
                return
            assert time.monotonic() < deadline and self.p.poll() is None, self.text.decode(errors='replace')
            for key, _ in self.selector.select(.1): self.text.extend(os.read(key.fd, 65536))
            assert b'panic:' not in self.text and b'vmcheck: FAIL' not in self.text, self.text.decode(errors='replace')

    def login(self, first=False, user='root', password=PASSWORD):
        if first:
            self.expect('First boot: set the root password')
            self.expect('New password: '); self.send(password)
            self.expect('Retype password: '); self.send(password)
        self.expect('login: '); self.send(user)
        self.expect('Password: '); self.send(password)
        self.expect('$ ')

    def run(self, command, output=None):
        self.send(command); self.expect(command + '\n')
        if output: self.expect(output)
        self.expect('$ ')

    def close(self):
        try:
            self.send(b'\x01x'); self.p.wait(timeout=5)
        except (OSError, subprocess.TimeoutExpired):
            self.p.kill(); self.p.wait()
        self.selector.close()
        (a.image.parent / f'rootfs-update-{self.label}-{a.cpus}cpu.log').write_bytes(self.text)
        for secret in (PASSWORD, USERPASS, OTHERPASS): assert secret.encode() not in self.text, 'password echoed'


with tempfile.TemporaryDirectory(prefix='rootfs-update-guest-', dir=a.image.parent) as temporary:
    root = Path(temporary)
    seed, disk = root / 'seed.img', root / 'rootfs.img'
    shutil.copyfile(a.image, seed)
    subprocess.run([sys.executable, str(ROOT / 'tools/create_rootfs.py'), '--seed', str(seed), '--output', str(disk), '--fresh'], check=True)
    guest = Guest(disk, 'before')
    try:
        guest.login(first=True)
        guest.run('echo ROOT_RETAINED > /root/note')
        guest.run('useradd alice', 'uid=1000')
        guest.send('passwd alice'); guest.expect('New password: '); guest.send(USERPASS)
        guest.expect('Retype password: '); guest.send(USERPASS); guest.expect('password updated'); guest.expect('$ ')
        guest.run('echo HOME_RETAINED > /home/alice/note')
        guest.run('chown alice /home/alice/note')
        guest.run('chmod 640 /home/alice/note')
        guest.run('ln -s note /home/alice/link')
        guest.run('sync')
    finally: guest.close()
    image = Image(disk)
    top = image.entries(image.root); binary = top[b'bin']; etc = top[b'etc']
    shadow = image.read(image.entries(etc)[b'shadow'])
    image.unlink_file(binary, b'echo')
    stale = image.create(binary, b'echo', FILE, (0, 0, 0o755)); image.append(stale, b'OBSOLETE_PROGRAM\n')
    tests = image.entries(top[b'usr'])[b'tests']; image.unlink_file(tests, b'vmcheck')
    disk.write_bytes(image.data)
    before = disk.read_bytes()
    subprocess.run([sys.executable, str(ROOT / 'tools/update_rootfs.py'), '--seed', str(seed), '--image', str(disk), '--manifest', str(a.manifest)], check=True)
    backup = list(root.glob('rootfs.img.before-update-*.img'))
    assert len(backup) == 1 and backup[0].read_bytes() == before
    updated = Image(disk)
    assert updated.read(updated.entries(etc)[b'shadow']) == shadow
    assert updated.attributes(updated.entries(binary)[b'passwd']) == (0, 0, 0o4755)
    guest = Guest(disk, 'after')
    try:
        guest.login(); assert b'First boot:' not in guest.text
        guest.run('pwd', '/root\n')
        guest.run('echo PROGRAM_UPDATED', 'PROGRAM_UPDATED\n')
        guest.run('cat /root/note', 'ROOT_RETAINED\n')
        guest.run('su alice cat /home/alice/link', 'HOME_RETAINED\n')
        guest.run('vmcheck', 'vmcheck: regions, COW, copyout, lazy pages and protection passed')
        guest.send(b'\x04'); guest.login(user='alice', password=USERPASS)
        guest.run('pwd', '/home/alice\n'); guest.run('cat note', 'HOME_RETAINED\n')
        guest.run('sync')
    finally: guest.close()

    # Another recipient gets no accounts or files from the first owner's disk.
    other = root / 'another-owner.img'
    subprocess.run([sys.executable, str(ROOT / 'tools/create_rootfs.py'), '--seed', str(seed), '--output', str(other), '--fresh'], check=True)
    clean = Image(other); entries = clean.entries(clean.root)
    assert not {b'passwd', b'shadow', b'group'}.intersection(clean.entries(entries[b'etc']))
    assert set(clean.entries(entries[b'root'])) == {b'.', b'..'}
    guest = Guest(other, 'independent')
    try:
        guest.login(first=True, password=OTHERPASS)
        guest.run('pwd', '/root\n')
        guest.run('su alice id', 'su: unknown user')
        guest.run('echo OWN_ROOTFS', 'OWN_ROOTFS\n')
        guest.run('sync')
    finally: guest.close()
    assert Image(other).read(Image(other).entries(entries[b'etc'])[b'shadow']) != shadow

print('Rootfs program upgrade, retained credentials and independent owner tests passed')
