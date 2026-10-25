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
parser.add_argument('--process-groups', action='store_true')
parser.add_argument('--signals', action='store_true')
parser.add_argument('--foreground', action='store_true')
parser.add_argument('--jobs', action='store_true')
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
log_path = Path(args.image).parent / (f"jobs-{args.cpus}cpu.log" if args.jobs else f"foreground-{args.cpus}cpu.log" if args.foreground else f"signals-{args.cpus}cpu.log" if args.signals else f"process-groups-{args.cpus}cpu.log" if args.process_groups else f"tty-{args.cpus}cpu.log" if args.tty else f"process-scaling-{args.cpus}cpu.log" if args.process_scaling else "user-regression.log" if args.regression else f"boot-smoke-{args.ram_mib}MiB.log")
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
                if b'jobtest: FAIL:' in transcript:
                    raise RuntimeError('job control contract failed')
                if b'fgtest: FAIL:' in transcript:
                    raise RuntimeError('foreground contract failed')
                if b'SOME TESTS FAILED' in transcript:
                    raise RuntimeError('guest regression test failed')
                if b'panic:' in transcript:
                    raise RuntimeError('kernel panic')
    raise RuntimeError(f'timeout waiting for {token!r}')

def foreground_checks():
    global cursor

    def begin(command):
        global cursor
        cursor = len(transcript)
        start = cursor
        process.stdin.write((command + '\n').encode())
        process.stdin.flush()
        return start

    def markers(modes, start):
        patterns = [re.compile(rb'fgtest: ' + mode.encode() +
                               rb' ready pid=(\d+) pgid=(\d+)\n') for mode in modes]
        while True:
            matches = [pattern.search(transcript, start) for pattern in patterns]
            if all(matches):
                return [(int(match[1]), int(match[2])) for match in matches]
            expect('\n', timeout=15)

    def interrupt():
        global cursor
        cursor = len(transcript)
        process.stdin.write(b'\x03')
        process.stdin.flush()
        expect('$ ', timeout=15)

    begin('fgtest controls')
    expect('fgtest: controls passed sid=')
    expect('\n')
    session = int(re.findall(rb'fgtest: controls passed sid=(\d+)', transcript)[-1])
    expect('$ ')
    for mode in ['spin', 'read', 'spin']:
        start = begin('fgtest ' + mode)
        pid, group = markers([mode], start)[0]
        if pid != group:
            raise RuntimeError('foreground command is not a group leader')
        interrupt()
        begin(f'fgtest absent {pid}')
        expect('fgtest: member absent\n')
        expect('$ ')

    start = begin('fgtest spin ; echo LIST_MUST_NOT_RUN')
    markers(['spin'], start)
    interrupt()
    if b'\nLIST_MUST_NOT_RUN\n' in transcript[start:]:
        raise RuntimeError('interrupted command list continued')
    start = begin('echo LIST_FIRST ; fgtest spin')
    markers(['spin'], start)
    interrupt()

    start = begin('fgtest source | fgtest sink')
    members = markers(['source', 'sink'], start)
    if members[0][0] == members[1][0] or members[0][1] != members[1][1]:
        raise RuntimeError('pipeline members do not share a process group')
    interrupt()
    for pid, _ in members:
        begin(f'fgtest absent {pid}')
        expect('fgtest: member absent\n')
        expect('$ ')

    begin('cat')
    expect('cat\n')
    process.stdin.write(b'FG_CAT_INPUT\n')
    process.stdin.flush()
    expect('FG_CAT_INPUT\n')  # input echo
    expect('FG_CAT_INPUT\n')  # cat output, proving it passed the launch barrier
    interrupt()

    start = begin('fgtest background &')
    background, _ = markers(['background'], start)[0]
    start = begin('fgtest spin')
    markers(['spin'], start)
    interrupt()
    begin(f'fgtest cleanup {background}')
    expect('fgtest: background survived\n')
    expect('$ ')

    # At an idle prompt SIGINT is ignored by the shell; its partial line is flushed.
    process.stdin.write(b'echo SHOULD_NOT_RUN')
    process.stdin.flush()
    expect('echo SHOULD_NOT_RUN')
    process.stdin.write(b'\x03')
    process.stdin.flush()
    expect('^C\n')
    begin('echo AFTER_IDLE_INTR')
    expect('AFTER_IDLE_INTR\n')
    expect('$ ')
    if b'SHOULD_NOT_RUN\n' in transcript:
        raise RuntimeError('Ctrl-C failed to flush the unfinished command')
    begin('fgtest controls')
    expect('fgtest: controls passed sid=')
    expect('\n')
    expect('$ ')
    if int(re.findall(rb'fgtest: controls passed sid=(\d+)', transcript)[-1]) != session:
        raise RuntimeError('shell session changed across interrupts')
    if transcript.count(b'init: starting sh') != 1:
        raise RuntimeError('Ctrl-C restarted the shell')

def jobs_checks():
    global cursor

    def begin(command):
        global cursor
        cursor = len(transcript)
        process.stdin.write((command + '\n').encode())
        process.stdin.flush()
        return cursor

    def ready(command, modes):
        start = begin(command)
        patterns = [re.compile(rb'fgtest: ' + mode.encode() + rb' ready pid=(\d+) pgid=(\d+)\n')
                    for mode in modes]
        while not all(pattern.search(transcript, start) for pattern in patterns):
            expect('\n', timeout=15)
        return start

    def stopped(command):
        global cursor
        cursor = len(transcript)
        process.stdin.write(b'\x1a')
        process.stdin.flush()
        expect('Stopped ' + command + '\n', timeout=15)
        expect('$ ')
        matches = re.findall(rb'\[(\d+)\] Stopped ' + re.escape(command.encode()) + rb'\n', transcript)
        return int(matches[-1])

    def front(number):
        begin(f'fg %{number}')
        expect(f'[{number}] Foreground ')
        expect('\n')

    def interrupt():
        global cursor
        cursor = len(transcript)
        process.stdin.write(b'\x03')
        process.stdin.flush()
        expect('$ ', timeout=15)

    def poll(number, state, command):
        for _ in range(20):
            start = begin('jobs')
            expect('$ ')
            if f'[{number}] {state} {command}\n'.encode() in transcript[start:]:
                return
            time.sleep(0.02)
        raise RuntimeError(f'job {number} never became {state}')

    begin('jobtest contracts')
    expect('jobtest: contracts passed\n', timeout=15)
    expect('$ ')
    ready('fgtest spin', ['spin'])
    number = stopped('fgtest spin')
    poll(number, 'Stopped', 'fgtest spin')
    begin(f'bg %{number}')
    expect(f'[{number}] Running fgtest spin\n')
    expect('$ ')
    front(number)
    interrupt()

    begin('cat')
    expect('cat\n')
    process.stdin.write(b'BEFORE_STOP\n')
    process.stdin.flush()
    expect('BEFORE_STOP\n')
    expect('BEFORE_STOP\n')
    number = stopped('cat')
    begin(f'bg %{number}')
    expect(f'[{number}] Running cat\n')
    expect('$ ')
    poll(number, 'Stopped', 'cat')  # SIGTTIN: a background reader stops again.
    front(number)
    process.stdin.write(b'AFTER_RESUME\n')
    process.stdin.flush()
    expect('AFTER_RESUME\n')
    expect('AFTER_RESUME\n')
    interrupt()

    pipeline = 'fgtest source | fgtest sink'
    ready(pipeline, ['source', 'sink'])
    number = stopped(pipeline)
    begin(f'bg %{number}')
    expect(f'[{number}] Running ' + pipeline + '\n')
    expect('$ ')
    front(number)
    interrupt()

    begin('jobtest raw')
    expect('jobtest: raw ready\n')
    number = stopped('jobtest raw')
    poll(number, 'Stopped', 'jobtest raw')
    front(number)
    interrupt()  # shell termios must be restored after both stop and termination.

    ready('fgtest spin', ['spin'])
    older = stopped('fgtest spin')
    ready('fgtest spin', ['spin'])
    newer = stopped('fgtest spin')
    begin(f'bg %{older}')
    expect(f'[{older}] Running fgtest spin\n')
    expect('$ ')
    front(older)
    interrupt()
    begin('fg')
    expect(f'[{newer}] Foreground fgtest spin\n')
    interrupt()

    start = begin('jobtest exit &')
    expect('Running jobtest exit\n')
    expect('$ ')
    number = int(re.search(rb'\[(\d+)\] Running jobtest exit\n', transcript[start:])[1])
    poll(number, 'Done', 'jobtest exit')
    start = begin('jobs')
    expect('$ ')
    if re.search(rb'\[\d+\] (Stopped|Running) ', transcript[start:]):
        raise RuntimeError('jobs were not retired')
    begin('echo JOBS_DONE')
    expect('JOBS_DONE\n')
    expect('$ ')
    if transcript.count(b'init: starting sh') != 1:
        raise RuntimeError('job control restarted the shell')

try:
    expect('physical allocator: ' + args.allocator + '\n')
    expect('$ ')
    if args.jobs:
        jobs_checks()
    if args.foreground:
        foreground_checks()
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
    if args.process_groups:
        checks = [('grouptest', 'grouptest: all tests succeeded')]
    if args.signals:
        checks = [('signaltest', 'signaltest: all tests succeeded'),
                  ('ttytest', 'ttytest: all tests succeeded'),
                  ('echo SIGNAL_SHELL_OK', 'SIGNAL_SHELL_OK\n')]
    if args.foreground or args.jobs:
        checks = []
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
