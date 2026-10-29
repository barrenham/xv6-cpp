#!/usr/bin/env python3
"""Exercise unchanged 1000-child fork tests and time their actual test phase."""
import argparse
import json
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import tempfile
import time

from console_login import login_root


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', required=True, type=Path)
    parser.add_argument('--image', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--cpus', type=int, default=1)
    parser.add_argument('--repeat', type=int, default=1)
    args = parser.parse_args()
    if args.repeat < 1 or args.cpus < 1:
        parser.error('cpus and repeat must be positive')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    transcript = bytearray()
    timings = []
    cursor = 0
    with tempfile.TemporaryDirectory(prefix='fork-progress-', dir=args.output.parent) as tmp:
        image = Path(tmp) / 'fs.img'
        shutil.copyfile(args.image, image)
        command = [args.qemu, '-machine', 'virt', '-bios', 'none',
                   '-kernel', str(args.kernel), '-m', '128M', '-smp', str(args.cpus),
                   '-nographic', '-snapshot', '-global', 'virtio-mmio.force-legacy=false',
                   '-drive', f'file={image},if=none,format=raw,id=x0',
                   '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
        guest = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT, bufsize=0,
                                 env=dict(os.environ, TMPDIR=str(Path(tmp).resolve())))
        selector = selectors.DefaultSelector()
        selector.register(guest.stdout, selectors.EVENT_READ)

        def expect(text, timeout=30):
            nonlocal cursor
            token = text.encode()
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                at = transcript.find(token, cursor)
                if at >= 0:
                    cursor = at + len(token)
                    return
                if guest.poll() is not None:
                    raise RuntimeError(f'QEMU exited with {guest.returncode}')
                for key, _ in selector.select(.1):
                    data = os.read(key.fd, 65536)
                    if not data:
                        continue
                    transcript.extend(data)
                    for failure in (b'panic:', b'SOME TESTS FAILED', b'FAILED -- lost',
                                    b'wait stopped early', b'wait got too many',
                                    b'no fork at all!', b'runtest: fork error'):
                        if failure in transcript:
                            raise RuntimeError(f'guest failure: {failure.decode()}')
            raise TimeoutError(f'waiting for {text!r}')

        try:
            login_root(expect, guest)
            for iteration in range(args.repeat):
                for command, start_marker, finish_marker in (
                        ('forktest', 'fork test\n', 'fork test OK\n'),
                        ('usertests forktest', 'test forktest: ', 'OK\n')):
                    cursor = len(transcript)
                    begin = time.monotonic()
                    guest.stdin.write((command + '\n').encode())
                    guest.stdin.flush()
                    expect(start_marker, timeout=120)
                    test_begin = time.monotonic()
                    expect(finish_marker, timeout=120)
                    test_end = time.monotonic()
                    if command.startswith('usertests'):
                        expect('ALL TESTS PASSED', timeout=120)
                    expect('$ ', timeout=20)
                    end = time.monotonic()
                    result = dict(command=command, cpus=args.cpus, iteration=iteration,
                                  setup_seconds=round(test_begin - begin, 3),
                                  test_seconds=round(test_end - test_begin, 3),
                                  cleanup_seconds=round(end - test_end, 3),
                                  total_seconds=round(end - begin, 3))
                    timings.append(result)
                    print(json.dumps(result), flush=True)
        finally:
            guest.terminate()
            try:
                guest.wait(timeout=5)
            except subprocess.TimeoutExpired:
                guest.kill()
                guest.wait()
            selector.close()
            args.output.with_suffix('.log').write_bytes(transcript)
            args.output.write_text(json.dumps(timings, indent=2) + '\n')


if __name__ == '__main__':
    main()
