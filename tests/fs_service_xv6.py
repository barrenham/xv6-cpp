#!/usr/bin/env python3
"""Exercise the production supervisor twice on a private xv6 root disk."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import tempfile
import time

from console_login import login_root, PASSWORD


def boot(args, disk, phase):
    command = [args.qemu, '-machine', 'virt', '-bios', 'none',
               '-kernel', str(args.kernel), '-initrd', str(args.initrd),
               '-m', '128M', '-smp', str(args.cpus), '-nographic',
               '-global', 'virtio-mmio.force-legacy=false',
               '-object', 'rng-random,id=rng0,filename=/dev/urandom',
               '-device', 'virtio-rng-device,rng=rng0,bus=virtio-mmio-bus.2',
               '-drive', f'file={disk},if=none,format=raw,id=x0',
               '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
    process = subprocess.Popen(command, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               env=dict(os.environ, TMPDIR=str(disk.parent)))
    output = bytearray()
    cursor = 0
    log = args.kernel.parent / f'fs-service-xv6-{args.cpus}cpu-{phase}.log'
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)

        def send(text):
            process.stdin.write((text + '\n').encode())
            process.stdin.flush()

        def expect(token, timeout=60):
            nonlocal cursor
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                for failure in (b'xv6fstest: FAIL:', b'panic:', b'fsboot: cannot',
                                b'usertrap(): unexpected'):
                    start = output.find(failure)
                    if start >= 0 and b'\n' in output[start:]:
                        raise RuntimeError(output[start:].split(b'\n', 1)[0].decode(errors='replace'))
                position = output.find(token.encode(), cursor)
                if position >= 0:
                    cursor = position + len(token)
                    return
                if process.poll() is not None:
                    raise RuntimeError('QEMU exited before contract passed')
                for key, _ in selector.select(0.2):
                    data = os.read(key.fd, 65536)
                    if data:
                        output.extend(data)
                        log.write_bytes(output)
            raise RuntimeError(f'timeout waiting for {token!r}')

        try:
            if phase == 'first':
                login_root(expect, process)
            else:
                expect('login: ')
                send('root')
                expect('Password: ')
                send(PASSWORD)
                expect('root:/root$ ')
            send(f'xv6fstest {phase}')
            expect(f'xv6fstest: PASS: {phase} boot')
            expect('$ ')
            for backend, path in (('xv6', '/'), ('devfs', '/dev'),
                                  ('proc', '/proc'), ('ramfs', '/tmp')):
                marker = f'fsboot: {backend} mounted at {path}, server pid='.encode()
                if marker not in output:
                    raise RuntimeError(f'missing independent {backend} service')
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            process.stdin.close()
            process.stdout.close()
            log.write_bytes(output)
            print(output.decode(errors='replace'), end='')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--initrd', type=Path, required=True)
    parser.add_argument('--image', type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-service-xv6-') as directory:
        disk = Path(directory) / 'root.img'
        shutil.copyfile(args.image, disk)
        for phase in ('first', 'second'):
            boot(args, disk, phase)
    print(f'userspace xv6 root persistence ({args.cpus} CPUs): passed')


if __name__ == '__main__':
    main()
