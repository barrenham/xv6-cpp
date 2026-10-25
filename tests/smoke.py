#!/usr/bin/env python3
"""Boot an isolated disk snapshot and exercise C++ ABI, shell, VM and SMP."""
import argparse
import os
import selectors
import subprocess
import sys
import time
import tempfile
import shutil
import re
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--qemu', required=True)
parser.add_argument('--kernel', required=True)
parser.add_argument('--image', required=True)
parser.add_argument('--regression', action='store_true')
parser.add_argument('--process-scaling', action='store_true')
parser.add_argument('--tty', action='store_true')
parser.add_argument('--cpus', type=int, default=3)
parser.add_argument('--ram-mib', type=int, default=128)
parser.add_argument('--boot-only', action='store_true')
parser.add_argument('--allocator', choices=['buddy', 'freelist'], default='buddy')
args = parser.parse_args()
temporary = tempfile.TemporaryDirectory(prefix='xv-qemu-', dir=Path(args.image).parent)
test_image = Path(temporary.name) / "fs.img"
shutil.copyfile(args.image, test_image)
qemu_env = dict(os.environ, TMPDIR=temporary.name)
command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
           '-m', f'{args.ram_mib}M', '-smp', str(args.cpus), '-nographic', '-snapshot',
           '-global', 'virtio-mmio.force-legacy=false',
           '-drive', f'file={test_image},if=none,format=raw,id=x0',
           '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, bufsize=0, env=qemu_env)
selector = selectors.DefaultSelector()
selector.register(process.stdout, selectors.EVENT_READ)
transcript = bytearray()
log_path = Path(args.image).parent / (f"tty-{args.cpus}cpu.log" if args.tty else f"process-scaling-{args.cpus}cpu.log" if args.process_scaling else "user-regression.log" if args.regression else f"boot-smoke-{args.ram_mib}MiB.log")
log_file = log_path.open("wb")
cursor = 0

def expect(token, timeout=30):
    global cursor
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        found = transcript.find(token.encode(), cursor)
        if found != -1:
            cursor = found + len(token.encode())
            return
        if process.poll() is not None:
            raise RuntimeError(f'QEMU exited with {process.returncode}')
        for key, _ in selector.select(timeout=0.2):
            data = os.read(key.fd, 65536)
            if data:
                log_file.write(data)
                log_file.flush()
                transcript.extend(data)
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
                if b'SOME TESTS FAILED' in transcript:
                    raise RuntimeError('guest regression test failed')
                if b'panic:' in transcript:
                    raise RuntimeError('kernel panic')
    raise RuntimeError(f'timeout waiting for {token!r}')

try:
    expect('physical allocator: ' + args.allocator + '\n')
    expect('$ ')
    if args.tty:
        process.stdin.write(b'ttytest interactive\n')
        process.stdin.flush()
        expect('ttytest: raw ready\n')
        process.stdin.write(b'\x00\x04\r')
        process.stdin.flush()
        expect('ttytest: canonical ready\n')
        process.stdin.write(b'ab\x7fc\r')
        process.stdin.flush()
        expect('ttytest: eof ready\n')
        process.stdin.write(b'\x04')
        process.stdin.flush()
        expect('ttytest: all tests succeeded\n')
        expect('$ ')
        process.stdin.write(b'stty raw min 1 time 0\n')
        process.stdin.flush()
        expect('$ ')
        process.stdin.write(b'stty sane\n')
        process.stdin.flush()
        expect('$ ')
        process.stdin.write(b'stty -a\n')
        process.stdin.flush()
        expect('icanon echo -onlcr min 1 time 0\n')
        expect('$ ')
    ram = re.search(rb'RAM: base=0x[0-9a-f]+ bytes=(\d+) metadata=(\d+) usable_pages=(\d+)', transcript)
    if not ram or int(ram[1]) != args.ram_mib * 1024 * 1024:
        raise RuntimeError('kernel RAM discovery does not match QEMU RAM size')
    mappings = re.search(rb'kernel mappings: 1GiB=(\d+) 2MiB=(\d+) 4KiB=(\d+) table_pages=(\d+)', transcript)
    if not mappings or int(mappings[4]) > 16 or int(mappings[2]) == 0:
        raise RuntimeError('kernel direct map uses too many page-table pages')
    if args.ram_mib >= 2048 and int(mappings[1]) == 0:
        raise RuntimeError('large RAM did not use a 1 GiB leaf')
    if b'direct map self-check: OK' not in transcript:
        raise RuntimeError('kernel mapping boundary self-check was not completed')
    checks = [('echo CPP_BOOT_OK', 'CPP_BOOT_OK\n'),
              ('cppdemo', 'cppdemo: RAII and move ownership OK'),
              ('pgtbltest', 'pgtbltest: all tests succeeded'),
              ('pgtblext', 'pgtblext: all tests succeeded')]
    if args.allocator == 'freelist':
        # The page-table lab's superpage assertions require the buddy capability.
        checks = checks[:2] + [('usertests sbrkbasic', 'ALL TESTS PASSED')]
    if args.boot_only:
        checks = checks[:2] + [('pgtblext', 'pgtblext: all tests succeeded')]
    if args.process_scaling:
        checks = [('procext', 'procext: 96 concurrent children, wake/kill/reuse OK')]
    if args.regression:
        checks = [('usertests -q', 'ALL TESTS PASSED')]
    for cmd, token in checks:
        process.stdin.write((cmd + '\n').encode())
        process.stdin.flush()
        expect(token, timeout=240 if args.regression else 45)
        expect('$ ')
    print('\nSmoke test passed')
finally:
    log_file.close()
    selector.close()
    process.terminate()
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
    temporary.cleanup()
