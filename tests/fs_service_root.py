#!/usr/bin/env python3
"""Boot a user-served ext3 root twice on one private persistent disk."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import tempfile
import time


def boot(args, disk, run):
    command = [args.qemu, '-machine', 'virt', '-bios', 'none',
               '-kernel', str(args.kernel), '-initrd', str(args.program),
               '-m', '128M', '-smp', str(args.cpus), '-nographic',
               '-global', 'virtio-mmio.force-legacy=false',
               '-drive', f'file={disk},if=none,format=raw,id=x0',
               '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
    process = subprocess.Popen(command, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               env=dict(os.environ, TMPDIR=str(disk.parent)))
    output = bytearray()
    passed = b'fsrootboottest: PASS: persistent root served entirely by a user process\n'
    log = args.kernel.parent / f'fs-service-root-{args.cpus}cpu-{run}.log'
    try:
        with selectors.DefaultSelector() as selector:
            selector.register(process.stdout, selectors.EVENT_READ)
            deadline = time.monotonic() + 90
            while time.monotonic() < deadline:
                for key, _ in selector.select(0.2):
                    data = os.read(key.fd, 65536)
                    if data:
                        output.extend(data)
                        log.write_bytes(output)
                for failure in (b'FAIL:', b'panic:', b'usertrap()'):
                    position = output.find(failure)
                    if position >= 0 and b'\n' in output[position:]:
                        raise RuntimeError('root filesystem bootstrap failed')
                if passed in output:
                    break
                if process.poll() is not None:
                    raise RuntimeError('QEMU exited before root contract passed')
            else:
                raise RuntimeError('user-served root timeout')
        expected = b'persistent marker created' if run == 1 else b'persistent marker recovered'
        if expected not in output or b'executable read through service' not in output:
            raise RuntimeError('persistence or executable loading was not exercised')
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
    parser.add_argument('--program', type=Path, required=True)
    parser.add_argument('--image', type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-fs-service-root-') as name:
        disk = Path(name) / 'root.img'
        shutil.copyfile(args.image, disk)
        for run in (1, 2):
            boot(args, disk, run)
        checker = shutil.which('e2fsck') or '/usr/sbin/e2fsck'
        result = subprocess.run([checker, '-fn', str(disk)], capture_output=True, text=True)
        print(result.stdout, end='')
        if result.returncode:
            print(result.stderr, end='')
            raise RuntimeError('offline ext3 consistency check failed')
    print(f'userspace root contract ({args.cpus} CPUs): passed')


if __name__ == '__main__':
    main()
