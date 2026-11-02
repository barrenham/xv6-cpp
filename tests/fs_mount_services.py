#!/usr/bin/env python3
"""Exercise automatic user FS instances and private optional disks in QEMU."""
import argparse
import os
from pathlib import Path
import re
import selectors
import shutil
import subprocess
import tempfile
import time

from console_login import login_root, PASSWORD


def boot(args, root, data, phase):
    command = [args.qemu, '-machine', 'virt', '-bios', 'none',
               '-kernel', str(args.kernel), '-initrd', str(args.initrd),
               '-m', '128M', '-smp', str(args.cpus), '-nographic',
               '-global', 'virtio-mmio.force-legacy=false',
               '-object', 'rng-random,id=rng0,filename=/dev/urandom',
               '-device', 'virtio-rng-device,rng=rng0,bus=virtio-mmio-bus.2',
               '-drive', f'file={root},if=none,format=raw,id=x0',
               '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
    if data:
        command += ['-drive', f'file={data},if=none,format=raw,id=x1',
                    '-device', 'virtio-blk-device,drive=x1,bus=virtio-mmio-bus.1']
    process = subprocess.Popen(command, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               env=dict(os.environ, TMPDIR=str(root.parent)))
    output = bytearray()
    cursor = 0
    log = args.kernel.parent / f'fs-manager-{args.data_format}-{args.cpus}cpu-{phase}.log'
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)

        def send(text):
            process.stdin.write((text + '\n').encode())
            process.stdin.flush()

        def expect(token, timeout=60):
            nonlocal cursor
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                for failure in (b'fsmgrtest: FAIL:', b'panic:', b'usertrap(): unexpected'):
                    start = output.find(failure)
                    if start >= 0 and b'\n' in output[start:]:
                        raise RuntimeError(output[start:].split(b'\n', 1)[0].decode(errors='replace'))
                position = output.find(token.encode(), cursor)
                if position >= 0:
                    cursor = position + len(token)
                    return
                if process.poll() is not None:
                    raise RuntimeError('QEMU exited before mount contract passed')
                for key, _ in selector.select(0.2):
                    chunk = os.read(key.fd, 65536)
                    if chunk:
                        output.extend(chunk)
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
            mode = args.data_format if data else 'basic'
            send(f'fsmgrtest {mode} {phase}')
            expect(f'fsmgrtest: PASS: {mode}', timeout=100)
            expect('$ ')
            if not data or phase == 'second':
                manager = re.search(rb'fsboot: mount manager ready, pid=(\d+)', output)
                if not manager:
                    raise RuntimeError('no mount manager startup identity')
                send(f'fsmgrtest cancel {int(manager[1])}')
                expect('fsmgrtest: PASS: stop')
                expect('$ ')
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
    parser.add_argument('--mkfs', required=True)
    parser.add_argument('--data-format', choices=('none', 'ext3', 'xv6'), default='none')
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-managed-fs-') as directory:
        directory = Path(directory)
        root = directory / 'root.img'
        shutil.copyfile(args.image, root)
        data = None
        if args.data_format != 'none':
            data = directory / 'data.img'
            manifest = directory / 'empty.tsv'
            manifest.write_text('')
            subprocess.run([args.mkfs, str(data), '--manifest', str(manifest),
                            '--filesystem', args.data_format], check=True)
        for phase in (('first', 'second') if data else ('first',)):
            boot(args, root, data, phase)
        if args.data_format == 'ext3':
            checker = shutil.which('e2fsck') or '/usr/sbin/e2fsck'
            subprocess.run([checker, '-fn', str(data)], check=True)
    print(f'userspace mount manager ({args.data_format}, {args.cpus} CPUs): passed')


if __name__ == '__main__':
    main()
