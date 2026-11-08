#!/usr/bin/env python3
"""Bounded same-boot mixed workload; records resource drift and verifies reboot data."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

from console_login import PASSWORD, login_root
from fs_elf_plugins import Guest
from fs_live_install import Server


def plain(text):
    return re.sub(r'\x1b\[[0-9;]*m', '', text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('kernel', 'bootstrap', 'image', 'output'):
        parser.add_argument('--' + option, type=Path, required=True)
    parser.add_argument('--qemu', default='qemu-system-riscv64')
    parser.add_argument('--cpus', type=int, default=5)
    parser.add_argument('--seconds', type=int, default=600)
    parser.add_argument('--memory-drift-kib', type=int, default=1024)
    args = parser.parse_args()
    if args.seconds < 60 or args.memory_drift_kib < 0:
        parser.error('duration must be >=60 seconds and memory tolerance nonnegative')
    args.output.mkdir(parents=True, exist_ok=False)
    image = args.output / 'root.img'
    shutil.copyfile(args.image, image)
    http = args.output / 'http'
    http.mkdir()
    payload = bytes(range(256)) * 4096
    (http / 'payload.bin').write_bytes(payload)
    server = Server(http)
    records = []
    result = {'status': 'running', 'cpus': args.cpus, 'requested_seconds': args.seconds,
              'memory_drift_limit_kib': args.memory_drift_kib,
              'kernel_sha256': hashlib.sha256(args.kernel.read_bytes()).hexdigest(),
              'harness_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              'samples': records}
    def save():
        (args.output / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
    save()
    guest = Guest(args, image, args.output / 'console.log', network=True)
    started = None
    sequence = 0
    def command(text, expect=None, timeout=60, **kwargs):
        nonlocal sequence
        guest.cursor = len(guest.output)
        begin = guest.cursor
        if text.rstrip().endswith('&'):
            guest.send(text)
            # A redraw prompt can belong to an older job notification. Wait
            # for this specific command's launch acknowledgement first.
            guest.expect('Running ' + text.rstrip()[:-1].rstrip() + '\n', timeout=30)
            guest.expect('$ ', timeout=20)
            return plain(guest.output[begin:guest.cursor].decode(errors='replace'))
        # Job-completion notifications can redraw the UART input line. Frame
        # foreground output by a standalone sentinel, not exact input echo.
        sequence += 1
        token = f'RELIABILITY_DONE_{sequence}'
        guest.send(text + '; echo ' + token)
        guest.expect('\n' + token + '\n', timeout=timeout)
        end = guest.cursor
        guest.expect('$ ', timeout=20)
        body = plain(guest.output[begin:end].decode(errors='replace'))
        if expect is not None:
            assert expect in body, (text, expect, body)
        assert not any(mark in body for mark in ('cannot open', 'cannot execute', 'exec failed')), body
        return body
    def sample(cycle):
        memory = command('cat /proc/meminfo')
        fields = {key: int(value) for key, value in re.findall(r'^(\w+):\s+(\d+)', memory, re.M)}
        uptime = command('cat /proc/uptime')
        seconds = re.search(r'^\s*(\d+(?:\.\d+)?)\s+\d+(?:\.\d+)?\s*$', uptime, re.M)
        assert seconds, uptime
        tasks = command('env NO_COLOR=1 ps -T', expect='CPU_MS')
        rows = [line.split() for line in tasks.splitlines() if re.match(r'^\s*\d+\s+\d+\s+[A-Z?]\s+', line)]
        assert rows and not any(row[-1] in ('schedswitch', 'fspoolbench', 'curl', 'ping') for row in rows), tasks
        record = {'cycle': cycle, 'elapsed_seconds': round(time.monotonic() - started, 3),
                  'free_kib': fields['MemFree'], 'cache_kib': fields['Cached'],
                  'noncache_used_kib': fields['MemTotal'] - fields['MemFree'] - fields['Cached'],
                  'tasks': len(rows), 'processes': len({row[0] for row in rows}),
                  'guest_uptime_seconds': float(seconds[1]),
                  'cache_pages': fields['FileCachePages'], 'cache_pending': fields['FileCachePending']}
        records.append(record)
        save()
        print(json.dumps(record), flush=True)
    try:
        login_root(guest.expect, guest.process)
        command('netctl dhcp wait eth0 10', expect='DHCP BOUND')
        command('echo initial > /root/reliability-download')
        started = time.monotonic()
        cycle = 0
        while time.monotonic() - started < args.seconds or cycle < 8:
            cycle += 1
            command('rm /root/reliability-download')
            command('schedctl ' + ('mlfq', 'round-robin', 'cfs')[(cycle - 1) % 3])
            command('/usr/tests/schedswitch > /tmp/reliability-scheduler.log &')
            command('ping -q -c 4 -i .02 -W 2 10.0.2.2 > /tmp/reliability-ping.log &')
            command(f'curl -fsS -m 60 -o /root/reliability-download {server.url}/payload.bin > /tmp/reliability-curl.log &')
            benchmark = command('/usr/tests/fspoolbench', expect=' PASS', timeout=180)
            assert 'fspoolbench: FAIL' not in benchmark, benchmark
            deadline = time.monotonic() + 180
            while True:
                jobs = command('env NO_COLOR=1 ps -T', expect='CPU_MS')
                if not re.search(r'\b(schedswitch|fspoolbench|curl|ping)\s*$', jobs, re.M):
                    break
                assert time.monotonic() < deadline, jobs
                time.sleep(.2)
            command('cat /tmp/reliability-scheduler.log', expect='schedswitchtest: PASS')
            command('cat /tmp/reliability-ping.log', expect='4 packets transmitted, 4 received')
            size = command('wc /root/reliability-download')
            assert re.search(r'^\s*\d+\s+\d+\s+1048576\s+/root/reliability-download', size, re.M), size
            command(f'echo ROUND_{cycle} > /root/reliability-marker')
            command('schedctl cfs', expect='Policy:    cfs')
            command('sync')
            # Give the mount supervisor a chance to reap the removed FS worker.
            time.sleep(.3)
            sample(cycle)
            records[-1]['benchmark'] = benchmark.strip()
            save()
        stable = records[2:]
        assert len(stable) >= 6
        baseline = min(row['noncache_used_kib'] for row in stable[:3])
        final = min(row['noncache_used_kib'] for row in stable[-3:])
        result['rounds'] = cycle
        result['same_boot_seconds'] = round(time.monotonic() - started, 3)
        result['noncache_baseline_kib'] = baseline
        result['noncache_final_kib'] = final
        result['noncache_drift_kib'] = final - baseline
        result['task_count_min'] = min(row['tasks'] for row in stable)
        result['task_count_max'] = max(row['tasks'] for row in stable)
        assert final - baseline <= args.memory_drift_kib, ('noncache memory grew', baseline, final)
        result['task_count_drift'] = min(row['tasks'] for row in stable[-3:]) - min(row['tasks'] for row in stable[:3])
        assert result['task_count_drift'] <= 4, ('task count grew', stable)
        result['same_boot_seconds'] = round(time.monotonic() - started, 3)
        guest.cursor = len(guest.output)
        guest.send('reboot')
        guest.expect('reboot: filesystem sync complete; rebooting')
        guest.expect('login: ', timeout=60)
        guest.send('root'); guest.expect('Password: ')
        guest.send(PASSWORD); guest.expect('root:/root$ ', timeout=60)
        command('cat /root/reliability-marker', expect=f'ROUND_{cycle}')
        command('schedctl', expect='Policy:    cfs')
        command('netctl dhcp wait eth0 10', expect='DHCP BOUND')
        command('ping -q -c 1 -W 2 10.0.2.2', expect='1 packets transmitted, 1 received')
        guest.send('poweroff')
        tail, _ = guest.process.communicate(timeout=25)
        guest.output.extend(tail)
        assert guest.process.returncode == 0
        assert b'panic:' not in guest.output
        checked = subprocess.run(['e2fsck', '-fn', str(image)], capture_output=True, text=True)
        (args.output / 'e2fsck.log').write_text(checked.stdout + checked.stderr)
        assert checked.returncode == 0
        sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
        from ext3_image import Ext3Image
        disk = Ext3Image.from_bytes(image.read_bytes())
        node = disk.entries(disk.entries(disk.root)[b'root'])[b'reliability-download']
        assert disk.read(node) == payload, 'final download bytes differ after reboot and poweroff'
        result['rounds'] = cycle
        result['payload_sha256'] = hashlib.sha256(payload).hexdigest()
        result['status'] = 'passed'
        save()
    except BaseException as error:
        result['status'] = 'failed'
        result['error'] = repr(error)
        save()
        raise
    finally:
        guest.close()
        server.close()
    print('Reliability soak PASS', flush=True)


if __name__ == '__main__':
    main()
