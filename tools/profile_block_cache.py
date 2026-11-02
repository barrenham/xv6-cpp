#!/usr/bin/env python3
"""Compare instrumented linear and hashed caches using private QEMU disks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import selectors
import shutil
import subprocess
import sys
import tempfile
import time
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
from console_login import login_root
from qemu_boot import bootstrap_args


def cpu_seconds(pid):
    fields = Path(f'/proc/{pid}/stat').read_text().rsplit(')', 1)[1].split()
    return (int(fields[11]) + int(fields[12])) / os.sysconf('SC_CLK_TCK')


def guest_run(qemu, kernel, seed, cpus, log):
    with tempfile.TemporaryDirectory(prefix='xv-cache-profile-') as temporary:
        image = Path(temporary) / 'fs.img'
        shutil.copyfile(seed, image)
        command = [qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(kernel),
                   *bootstrap_args(kernel),
                   '-m', '128M', '-smp', str(cpus), '-nographic', '-snapshot',
                   '-global', 'virtio-mmio.force-legacy=false',
                   '-drive', f'file={image},if=none,format=raw,id=x0',
                   '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
                   '-object', 'rng-random,id=rng0,filename=/dev/urandom',
                   '-device', 'virtio-rng-device,rng=rng0,bus=virtio-mmio-bus.2']
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, bufsize=0,
                                   env=dict(os.environ, TMPDIR=temporary))
        selector = selectors.DefaultSelector()
        selector.register(process.stdout, selectors.EVENT_READ)
        transcript = bytearray()
        cursor = 0

        def expect(token, timeout=120):
            nonlocal cursor
            token = token.encode()
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                index = transcript.find(token, cursor)
                if index >= 0:
                    cursor = index + len(token)
                    return
                if process.poll() is not None:
                    raise RuntimeError(f'QEMU stopped: {process.returncode}')
                for key, _ in selector.select(.1):
                    transcript.extend(os.read(key.fd, 65536))
                    log.write_bytes(transcript)
                if b'panic:' in transcript or b'CACHEBENCH FAIL' in transcript:
                    raise RuntimeError('guest cache failure')
            raise TimeoutError(token)

        def send(command):
            process.stdin.write((command + '\n').encode())
            process.stdin.flush()

        try:
            login_root(expect, process)
            results = []
            for pattern in ['repeat', 'sequential', 'random', 'parallel']:
                start = len(transcript)
                wall_begin, cpu_begin = time.monotonic(), cpu_seconds(process.pid)
                send('/usr/tests/cachebench ' + pattern)
                expect('CACHEBENCH done\n')
                wall, cpu = time.monotonic() - wall_begin, cpu_seconds(process.pid) - cpu_begin
                match = re.search(rb'CACHEBENCH ' + pattern.encode() + rb' ([^\r\n]+)', transcript[start:])
                assert match, transcript[start:]
                counters = {key.decode(): int(value) for key, value in re.findall(rb'(\w+)=(\d+)', match[1])}
                assert counters['ops'] == 2048 and counters['hits'] + counters['misses'] >= 2048
                results.append(dict(pattern=pattern, wall_s=wall, qemu_cpu_s=cpu, **counters))
                expect('root:/$ ')
            return dict(cpus=cpus, results=results)
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            selector.close()
            log.write_bytes(transcript)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--image', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpus', type=int, nargs='+', default=[1, 5])
    parser.add_argument('--rounds', type=int, default=2)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    cases = ([('linear', args.baseline)] if args.baseline else []) + [('hashed', args.kernel)]
    report = {'sha256': {}, 'runs': [], 'timebase_hz': 10000000,
              'cpu_method': 'Linux QEMU process user+system; includes all threads; wall includes command startup and final counter printing'}
    for label, path in cases + [('image', args.image)]:
        report['sha256'][label] = hashlib.sha256(path.read_bytes()).hexdigest()
    # Read benchmarks use one identical prepared disk. Fixture construction is
    # outside the timed guest and never modifies the supplied seed.
    from create_rootfs import Image, FILE
    prepared = args.output.parent / 'prepared.img'
    shutil.copyfile(args.image, prepared)
    fixture = Image(prepared)
    for file in range(4):
        number = fixture.create(fixture.root, f'cachebench{file}.data'.encode(), FILE)
        payload = bytes((file * 17 + block * 13 + offset) % 251
                        for block in range(128) for offset in range(1024))
        fixture.append(number, payload)
    prepared.write_bytes(fixture.data)
    del fixture
    report['sha256']['prepared_image'] = hashlib.sha256(prepared.read_bytes()).hexdigest()
    for cpus in args.cpus:
        for round_number in range(args.rounds):
            for label, kernel in cases:
                print(f'{label}: {cpus} CPUs round {round_number + 1}', flush=True)
                result = guest_run(args.qemu, kernel, prepared, cpus,
                                   args.output.parent / f'{label}-{cpus}cpu-{round_number + 1}.log')
                report['runs'].append(dict(label=label, round=round_number + 1, **result))
                args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
