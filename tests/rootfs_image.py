#!/usr/bin/env python3
"""Check safe rootfs publication and legacy journal recovery off the guest path."""
import argparse
import fcntl
from pathlib import Path
import struct
import socket
import time
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from create_rootfs import Image, FILE

parser = argparse.ArgumentParser()
parser.add_argument('--image', required=True, type=Path)
parser.add_argument('--qemu')
args = parser.parse_args()
script = ROOT / 'tools/create_rootfs.py'


def create(seed, output, legacy=None, success=True, fresh=False):
    command = [sys.executable, str(script), '--seed', str(seed), '--output', str(output)]
    if fresh:
        command += ['--fresh']
    if legacy is not None:
        command += ['--import-data', str(legacy)]
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    assert (result.returncode == 0) == success, result.stdout.decode()


with tempfile.TemporaryDirectory(prefix='rootfs-contract-') as temporary:
    root = Path(temporary)
    output = root / 'rootfs.img'
    create(args.image, output)
    initial = output.read_bytes()
    create(root / 'missing-seed.img', output)
    assert output.read_bytes() == initial
    assert output.stat().st_mode & 0o777 == 0o600

    create(args.image, output, fresh=True, success=False)
    assert output.read_bytes() == initial
    personal = root / 'personal.img'
    private = Image(args.image)
    etc = private.entries(1)[b'etc']
    shadow = private.create(etc, b'shadow', FILE, (0, 0, 0o600))
    private.append(shadow, b'PRIVATE_ACCOUNT_DATA\n')
    personal.write_bytes(private.data)
    clean = root / 'clean.img'
    create(personal, clean, fresh=True, success=False)
    assert not clean.exists()
    create(args.image, clean, fresh=True)
    assert b'shadow' not in Image(clean).entries(Image(clean).entries(1)[b'etc'])

    malformed = root / 'bad.img'
    malformed.write_bytes(b'not a filesystem')
    failed = root / 'failed.img'
    create(malformed, failed, success=False)
    assert not failed.exists()

    # A disk in use must not be copied while its contents are changing.
    locker = subprocess.Popen([sys.executable, '-c',
        'import fcntl,sys; f=open(sys.argv[1],"r+b"); fcntl.lockf(f,fcntl.LOCK_EX); print("locked",flush=True); sys.stdin.read()',
        str(output)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        assert locker.stdout.readline() == b'locked\n'
        create(output, failed, success=False)
        assert not failed.exists()
    finally:
        locker.communicate()
    assert output.read_bytes() == initial

    if args.qemu:
        qmp = root / 'qmp.sock'
        guest = subprocess.Popen([args.qemu, '-machine', 'virt', '-bios', 'none',
            '-S', '-display', 'none', '-serial', 'none', '-monitor', 'none',
            '-qmp', f'unix:{qmp},server=on,wait=off',
            '-drive', f'file={output},if=none,format=raw,id=x0',
            '-device', 'virtio-blk-device,drive=x0'], stderr=subprocess.DEVNULL)
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
                    create(output, failed, success=False)
                    assert not failed.exists()
        finally:
            guest.terminate()
            guest.wait(timeout=5)
        assert output.read_bytes() == initial

    # A conflicting legacy tree must leave both the old disk and output intact.
    create(args.image, failed, args.image, success=False)
    assert not failed.exists()

    # Recover a committed legacy log in the private copy, preserving its source.
    volume = Image(args.image)
    inode = volume.create(1, b'journal-note', FILE)
    volume.append(inode, b'UNAPPLIED\n')
    destination = volume.block(inode, 0)
    struct.pack_into('<II', volume.data, volume.log * 1024, 1, destination)
    offset = (volume.log + 1) * 1024
    volume.data[offset:offset + 1024] = b'RECOVERED\n' + bytes(1014)
    source = root / 'journal.img'
    source.write_bytes(volume.data)
    original = source.read_bytes()
    recovered = Image(source)
    assert recovered.read(inode) == b'RECOVERED\n'
    # Use the recovered view as a seed, proving publication clears its journal.
    target = root / 'recovered.img'
    create(source, target)
    published = Image(target)
    assert published.read(published.entries(1)[b'journal-note']) == b'RECOVERED\n'
    assert struct.unpack_from('<I', target.read_bytes(), volume.log * 1024)[0] == 0
    assert source.read_bytes() == original

print('Rootfs publication and recovery contracts passed')
