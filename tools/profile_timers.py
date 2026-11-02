#!/usr/bin/env python3
"""Compare real poll wake latency and QEMU process CPU on private disk copies."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import selectors
import shutil
import statistics
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


def run(qemu, kernel, image, cpus, destination):
    with tempfile.TemporaryDirectory(prefix='xv-timers-') as temporary:
        disk = Path(temporary) / 'fs.img'
        shutil.copyfile(image, disk)
        command = [qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(kernel),
                   *bootstrap_args(kernel),
                   '-m', '128M', '-smp', str(cpus), '-nographic', '-snapshot',
                   '-global', 'virtio-mmio.force-legacy=false',
                   '-drive', f'file={disk},if=none,format=raw,id=x0',
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

        def expect(token, timeout=40):
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
                if b'panic:' in transcript:
                    raise RuntimeError('guest panic')
            raise TimeoutError(token)

        def send(command):
            process.stdin.write((command + '\n').encode())
            process.stdin.flush()

        try:
            login_root(expect, process)
            send('/usr/tests/timerbench idle')
            expect('TIMERBENCH idle-start\n')
            wall_start, cpu_start = time.monotonic(), cpu_seconds(process.pid)
            expect('TIMERBENCH idle-end\n')
            idle = {'wall_s': time.monotonic() - wall_start,
                    'qemu_cpu_s': cpu_seconds(process.pid) - cpu_start}
            expect('root:/$ ')
            start = len(transcript)
            wall_start, cpu_start = time.monotonic(), cpu_seconds(process.pid)
            send('/usr/tests/timerbench')
            expect('TIMERBENCH measured\n', timeout=45)
            wall, cpu = time.monotonic() - wall_start, cpu_seconds(process.pid) - cpu_start
            expect('TIMERBENCH done\n')
            samples = {}
            for interval, ticks in re.findall(rb'TIMERBENCH sample (\d+) (\d+)', transcript[start:]):
                samples.setdefault(int(interval), []).append(int(ticks) / 10000)
            assert set(samples) == {1, 5, 10, 25, 100}
            results = {}
            for interval, values in samples.items():
                assert len(values) == 20
                ordered = sorted(values)
                results[interval] = {'count': len(values), 'elapsed_ms': values,
                    'min_ms': ordered[0], 'median_ms': statistics.median(values),
                    'p95_ms': ordered[18], 'max_ms': ordered[-1],
                    'median_lateness_ms': statistics.median(values) - interval}
            return {'cpus': cpus, 'idle': idle, 'samples': results,
                    'batch_wall_s': wall, 'batch_qemu_cpu_s': cpu}
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            selector.close()
            destination.write_bytes(transcript)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--before', type=Path)
    parser.add_argument('--after', type=Path, required=True)
    parser.add_argument('--image', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=2)
    parser.add_argument('--cpus', type=int, nargs='+', default=[1, 5])
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    report = {'timebase_hz': 10000000, 'cpu_measure': 'Linux QEMU process user+system, all vCPU threads',
              'sha256': {}, 'runs': []}
    cases = ([('before', args.before)] if args.before else []) + [('after', args.after)]
    for label, path in cases + [('image', args.image)]:
        report['sha256'][label] = hashlib.sha256(path.read_bytes()).hexdigest()
    for cpus in args.cpus:
        for round_number in range(args.rounds):
            for label, kernel in cases:
                print(f'{label}: {cpus} CPUs round {round_number + 1}', flush=True)
                result = run(args.qemu, kernel, args.image, cpus,
                             args.output.parent / f'{label}-{cpus}cpu-{round_number + 1}.log')
                if label == 'after':
                    assert all(value >= int(interval) for interval, group in result['samples'].items()
                               for value in group['elapsed_ms']), 'poll returned before deadline'
                report['runs'].append(dict(label=label, round=round_number + 1, **result))
                args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
