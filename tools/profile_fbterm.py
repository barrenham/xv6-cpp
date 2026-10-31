#!/usr/bin/env python3
"""Profile fbterm stages on a private disk; never edits the persistent rootfs."""
import argparse
import json
import os
from pathlib import Path
import re
import selectors
import shutil
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
from console_login import login_root
from create_rootfs import Image, FILE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, default=Path('build/dev'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--qemu', default='qemu-system-riscv64')
    parser.add_argument('--cpus', type=int, choices=(1, 5), default=1)
    parser.add_argument('--lines', type=int, default=500)
    parser.add_argument('--rounds', type=int, default=2)
    parser.add_argument('--vnc-client', action='store_true')
    parser.add_argument('--no-serial-mirror', action='store_true')
    parser.add_argument('--modes', nargs='+', default=[
        'normal', 'no-mirror', 'no-present', 'no-paint', 'parse-only'])
    args = parser.parse_args()
    if not 1 <= args.lines <= 2000 or not 1 <= args.rounds <= 10:
        parser.error('lines must be 1..2000 and rounds 1..10')
    if any(mode not in ('normal', 'no-mirror', 'no-present', 'no-paint', 'parse-only')
           for mode in args.modes):
        parser.error('unsupported diagnostic mode')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    payload = ''.join(f'row-{i:05d}: fast terminal output 0123456789\n'
                      for i in range(args.lines)) + 'PROFILE_END\n'
    transcript = bytearray()
    cursor = 0
    results = []
    vnc = None
    vnc_thread = None
    traffic = dict(bytes=0, updates=0, error=None)
    stopping = threading.Event()
    guest = None
    selector = selectors.DefaultSelector()
    with tempfile.TemporaryDirectory(prefix='fbprofile-', dir=args.output.parent) as temp:
        temp = Path(temp)
        disk = temp / 'fs.img'
        shutil.copyfile(args.build / 'fs-test.img', disk)
        image = Image(disk)
        inode = image.create(image.root, b'burst.txt', FILE)
        image.append(inode, payload.encode())
        disk.write_bytes(image.data)
        del image
        cache = (args.build / 'CMakeCache.txt').read_text()
        width = int(re.search(r'^XV_DISPLAY_WIDTH:STRING=(\d+)$', cache, re.M)[1])
        height = int(re.search(r'^XV_DISPLAY_HEIGHT:STRING=(\d+)$', cache, re.M)[1])
        command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel',
                   str(args.build / 'kernel.elf'), '-m', '128M', '-smp', str(args.cpus),
                   '-display', 'none', '-serial', 'stdio', '-global',
                   'virtio-mmio.force-legacy=false', '-drive',
                   f'file={disk},if=none,format=raw,id=x0', '-device',
                   'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0', '-device',
                   f'virtio-gpu-device,bus=virtio-mmio-bus.3,xres={width},yres={height}']
        if args.vnc_client:
            with socket.socket() as reservation:
                reservation.bind(('127.0.0.1', 0))
                port = reservation.getsockname()[1]
            command += ['-vnc', f'127.0.0.1:{port-5900}']
        guest = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT, bufsize=0)
        selector.register(guest.stdout, selectors.EVENT_READ)

        def expect(token, timeout=180):
            nonlocal cursor
            token = token.encode()
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                found = transcript.find(token, cursor)
                if found >= 0:
                    cursor = found + len(token)
                    return
                if guest.poll() is not None:
                    raise RuntimeError('QEMU exited')
                for key, _ in selector.select(.05):
                    transcript.extend(os.read(key.fd, 65536))
                if b'panic:' in transcript:
                    raise RuntimeError('kernel panic')
            raise TimeoutError(f'waiting for {token[:120]!r}')

        def cpu_seconds():
            fields = Path(f'/proc/{guest.pid}/stat').read_text().rsplit(')', 1)[1].split()
            return (int(fields[11]) + int(fields[12])) / os.sysconf('SC_CLK_TCK')

        def receive(count):
            data = bytearray()
            while len(data) < count:
                part = vnc.recv(count - len(data))
                if not part:
                    raise EOFError('VNC disconnected')
                data.extend(part)
            traffic['bytes'] += count
            return bytes(data)

        def request(incremental):
            vnc.sendall(struct.pack('!BBHHHH', 3, incremental, 0, 0, width, height))

        def consume_vnc():
            try:
                request(0)
                while not stopping.is_set():
                    message = receive(1)[0]
                    if message == 0:
                        header = receive(3)
                        for _ in range(struct.unpack('!H', header[1:])[0]):
                            _, _, w, h, encoding = struct.unpack('!HHHHi', receive(12))
                            if encoding != 0:
                                raise RuntimeError(f'unexpected encoding {encoding}')
                            receive(w*h*4)
                        traffic['updates'] += 1
                        request(1)
                    elif message == 2:
                        continue
                    elif message == 3:
                        header = receive(7)
                        receive(struct.unpack('!I', header[3:])[0])
                    else:
                        raise RuntimeError(f'unexpected VNC message {message}')
            except Exception as error:
                if not stopping.is_set():
                    traffic['error'] = str(error)

        try:
            login_root(expect, guest)
            if args.vnc_client:
                vnc = socket.create_connection(('127.0.0.1', port), timeout=10)
                assert receive(12) == b'RFB 003.008\n'
                vnc.sendall(b'RFB 003.008\n')
                assert 1 in receive(receive(1)[0])
                vnc.sendall(b'\x01')
                assert receive(4) == b'\0\0\0\0'
                vnc.sendall(b'\x01')
                header = receive(24)
                assert header[4] == 32, '32-bit VNC pixel format required'
                receive(struct.unpack('!I', header[20:])[0])
                vnc.sendall(struct.pack('!BBHi', 2, 0, 1, 0))  # Raw encoding.
                vnc.settimeout(None)
                vnc_thread = threading.Thread(target=consume_vnc, daemon=True)
                vnc_thread.start()
            for round_number in range(args.rounds):
                modes = args.modes if round_number % 2 == 0 else args.modes[::-1]
                for mode in modes:
                    start_offset = len(transcript)
                    start_cpu = cpu_seconds()
                    start = time.monotonic()
                    start_traffic = traffic.copy()
                    option = '--no-serial-mirror ' if args.no_serial_mirror else ''
                    guest.stdin.write(f'env FBTERM_PROFILE={mode} fbterm {option}cat /burst.txt\n'.encode())
                    guest.stdin.flush()
                    expect('fbterm: connected')
                    expect('fbterm: disconnected')
                    elapsed = time.monotonic() - start
                    cpu = cpu_seconds() - start_cpu
                    expect('root:/$ ')
                    segment = bytes(transcript[start_offset:])
                    match = re.search(rb'FBPROFILE mode=(\S+) ([^\r\n]+)', segment)
                    assert match and match[1].decode() == mode, segment[-1500:]
                    stats = {key: int(value) for key, value in
                             re.findall(r'(\w+)=(\d+)', match[2].decode())}
                    expected = payload.replace('\n', '\r\n').encode()
                    assert stats['bytes'] == len(expected), stats
                    if not args.no_serial_mirror and mode not in ('no-mirror', 'parse-only'):
                        assert expected in segment, 'serial byte loss'
                        assert stats['mirrored'] == len(expected), stats
                    else:
                        assert stats['mirrored'] == 0, stats
                    assert not traffic['error'], traffic
                    result = dict(round=round_number+1, mode=mode, cpus=args.cpus,
                                  vnc_client=args.vnc_client, serial_mirror=not args.no_serial_mirror and mode not in (
                                      'no-mirror', 'parse-only'),
                                  host_wall_s=elapsed,
                                  qemu_cpu_s=cpu, ticks=stats,
                                  vnc_bytes=traffic['bytes']-start_traffic['bytes'],
                                  vnc_updates=traffic['updates']-start_traffic['updates'])
                    results.append(result)
                    print(json.dumps(result), flush=True)
                    args.output.write_text(json.dumps(dict(status='running', results=results), indent=2)+'\n')
            args.output.write_text(json.dumps(dict(status='passed', lines=args.lines,
                resolution=[width, height], timebase_hz=10000000,
                vnc_encoding='raw' if args.vnc_client else None, results=results), indent=2)+'\n')
        finally:
            stopping.set()
            if vnc:
                vnc.shutdown(socket.SHUT_RDWR)
                vnc.close()
            if vnc_thread:
                vnc_thread.join(timeout=5)
            guest.terminate()
            try:
                guest.wait(timeout=5)
            except subprocess.TimeoutExpired:
                guest.kill()
                guest.wait()
            selector.close()
            args.output.with_suffix('.serial.log').write_bytes(transcript)


if __name__ == '__main__':
    main()
