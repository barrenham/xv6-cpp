#!/usr/bin/env python3
"""Real E1000 DMA, ARP and UDP round trips against a local QEMU slirp peer."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import socket
import subprocess
import tempfile
import threading
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', required=True)
    parser.add_argument('--image', required=True)
    parser.add_argument('--cpus', type=int, default=3)
    parser.add_argument('--multi', action='store_true')
    parser.add_argument('--management', action='store_true')
    args = parser.parse_args()
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
        peer.bind(('127.0.0.1', 0))
        peer.settimeout(0.1)
        stop = threading.Event()
        received = []
        def echo():
            while not stop.is_set():
                try:
                    data, address = peer.recvfrom(4096)
                    received.append(data)
                    peer.sendto(data, address)
                except socket.timeout:
                    pass
        worker = threading.Thread(target=echo)
        worker.start()
        try:
            with tempfile.TemporaryDirectory(prefix='xv-net-', dir=Path(args.image).parent) as temporary:
                image = Path(temporary)/'fs.img'
                shutil.copyfile(args.image, image)
                command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
                           '-m', '128M', '-smp', str(args.cpus), '-nographic', '-snapshot',
                           '-global', 'virtio-mmio.force-legacy=false',
                           '-drive', f'file={image},if=none,format=raw,id=x0',
                           '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
                           '-netdev', 'user,id=net0', '-device',
                           'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56']
                if args.multi:
                    command += ['-netdev', 'user,id=net1,net=10.0.3.0/24,host=10.0.3.2,dhcpstart=10.0.3.15',
                                '-device', 'e1000,netdev=net1,bus=pcie.0,addr=05.0,mac=52:54:00:12:34:57']
                process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                           stderr=subprocess.STDOUT, bufsize=0)
                transcript = bytearray()
                log = Path(args.image).parent/f'{"netctl" if args.management else "multinet" if args.multi else "network"}-{args.cpus}cpu.log'
                selector = selectors.DefaultSelector()
                selector.register(process.stdout, selectors.EVENT_READ)
                cursor = 0
                def expect(token, timeout=25):
                    nonlocal cursor
                    deadline = time.monotonic()+timeout
                    while time.monotonic() < deadline:
                        position = transcript.find(token.encode(), cursor)
                        if position >= 0:
                            cursor = position+len(token)
                            return
                        if process.poll() is not None:
                            raise RuntimeError(f'QEMU exited: {process.returncode}')
                        for key, _ in selector.select(0.1):
                            data = os.read(key.fd, 65536)
                            transcript.extend(data)
                            if b'panic:' in transcript or b'nettest: FAIL:' in transcript or b'multinet: FAIL:' in transcript or b'netctltest: FAIL:' in transcript:
                                raise RuntimeError(transcript.decode(errors='replace'))
                    raise TimeoutError(f'waiting for {token}: '+transcript.decode(errors='replace'))
                try:
                    expect('e1000: ready')
                    expect('$ ')
                    if args.management:
                        commands = [('netctl link', 'eth0 UP'), ('netctl addr', '10.0.2.15/24'),
                                    ('netctl route', '0.0.0.0/0 dev eth0'),
                                    ('netctl link set eth0 down', None), ('netctl link', 'eth0 DOWN'),
                                    ('netctl link set eth0 up', None),
                                    ('netctl addr add 10.0.2.16/24 dev eth0', None),
                                    ('netctl addr', '10.0.2.16/24'),
                                    ('netctl addr del 10.0.2.16/24 dev eth0', None),
                                    ('netctl route add 192.0.2.0/24 dev eth0 via 10.0.2.2 metric 7', None),
                                    ('netctl route', '192.0.2.0/24 dev eth0 via 10.0.2.2 metric 7'),
                                    ('netctl route del 192.0.2.0/24 dev eth0 via 10.0.2.2 metric 7', None),
                                    ('netctl neigh flush eth0', None), ('netctl neigh', None),
                                    ('netctl stats', 'eth0 TX packets'),
                                    ('netctl addr add 999.0.0.1/24 dev eth0', 'usage:'),
                                    ('netctl route add 1.2.3.4/33 dev eth0', 'usage:')]
                        for text, token in commands:
                            process.stdin.write((text+'\n').encode()); process.stdin.flush()
                            if token: expect(token)
                            expect('$ ')
                    # Repeat in the same boot: exit must release all resources.
                    for _ in range(2):
                        program = 'netctltest' if args.management else 'multinet' if args.multi else 'nettest'
                        process.stdin.write(f'{program} {peer.getsockname()[1]}\n'.encode())
                        process.stdin.flush()
                        expect('netctltest: ALL PASSED' if args.management else 'multinet: two NICs and shared INTx passed' if args.multi else 'nettest: ALL PASSED')
                        expect('$ ')
                    assert len(received) >= (8 if args.management else 160 if args.multi else 170), len(received)
                    if not args.management:
                        assert any(len(p) == 1472 for p in received)
                    if not args.multi and not args.management:
                        assert b'' in received
                    print(f'network: {args.cpus} CPUs, {len(received)} datagrams, all passed')
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                    log.write_bytes(transcript)
                    selector.close()
        finally:
            stop.set()
            worker.join()


if __name__ == '__main__':
    main()
