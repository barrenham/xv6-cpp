#!/usr/bin/env python3
"""Exercise the pre-root userspace entry with an unformatted private disk."""
import argparse
import os
from pathlib import Path
import selectors
import subprocess
import tempfile
import time


def boot(args, directory, image, expected):
    disk = directory / 'unformatted.img'
    with disk.open('wb') as file:
        file.truncate(8 * 1024 * 1024)
    command = [args.qemu, '-machine', 'virt', '-bios', 'none',
               '-kernel', str(args.kernel), '-initrd', str(image),
               '-m', '128M', '-smp', str(args.cpus), '-nographic',
               '-global', 'virtio-mmio.force-legacy=false',
               '-drive', f'file={disk},if=none,format=raw,id=x0',
               '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
    process = subprocess.Popen(command, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               env=dict(os.environ, TMPDIR=str(directory)))
    transcript = bytearray()
    try:
        with selectors.DefaultSelector() as selector:
            selector.register(process.stdout, selectors.EVENT_READ)
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                for key, _ in selector.select(0.2):
                    data = os.read(key.fd, 65536)
                    if data:
                        transcript.extend(data)
                if expected in transcript:
                    break
                if b'fsboottest: FAIL:' in transcript or process.poll() is not None:
                    raise RuntimeError('bootstrap failed or QEMU exited')
            else:
                raise RuntimeError('bootstrap timeout')
        if expected.startswith(b'fsboottest: PASS:'):
            if b'panic:' in transcript or b'fsboottest: child console before root mount' not in transcript:
                raise RuntimeError('pre-root process lifecycle failed')
        elif b'fsboottest: entered userspace' in transcript:
            raise RuntimeError('malformed executable reached userspace')
    finally:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        process.stdin.close()
        process.stdout.close()
        log = args.kernel.parent / f'fs-bootstrap-{args.cpus}cpu-{image.name}.log'
        log.write_bytes(transcript)
        print(transcript.decode(errors='replace'), end='')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--program', type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-fsbootstrap-') as path:
        directory = Path(path)
        boot(args, directory, args.program,
             b'fsboottest: PASS: userspace bootstrap without a mounted filesystem\n')
        for name, contents in [('bad-magic', b'not an ELF executable'),
                               ('truncated-elf', args.program.read_bytes()[:64])]:
            image = directory / name
            image.write_bytes(contents)
            boot(args, directory, image, b'panic: exec')
    print(f'filesystem bootstrap ({args.cpus} CPUs): passed')


if __name__ == '__main__':
    main()
