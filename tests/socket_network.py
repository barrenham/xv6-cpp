#!/usr/bin/env python3
"""Exercise socket descriptors and TCP active/passive opens against real host peers."""
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
    args = parser.parse_args()
    stop = threading.Event()
    failures = []
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp, \
         socket.socket(socket.AF_INET, socket.SOCK_STREAM) as tcp, \
         socket.socket(socket.AF_INET, socket.SOCK_STREAM) as refused, \
         socket.socket(socket.AF_INET, socket.SOCK_STREAM) as forward:
        for peer in [udp, tcp, refused, forward]:
            peer.bind(('127.0.0.1', 0))
        tcp.listen(4)
        udp.settimeout(0.1)
        tcp.settimeout(0.1)
        refused_port = refused.getsockname()[1]
        forward_port = forward.getsockname()[1]
        # Keep the refused port bound but not listening.
        def udp_echo():
            while not stop.is_set():
                try:
                    data, address = udp.recvfrom(4096)
                    udp.sendto(data, address)
                except socket.timeout:
                    pass
                except Exception as error:
                    failures.append(error)
                    return
        def tcp_echo():
            while not stop.is_set():
                try:
                    connection, _ = tcp.accept()
                except socket.timeout:
                    continue
                try:
                    with connection:
                        connection.settimeout(10)
                        while data := connection.recv(1027):
                            connection.sendall(data)
                except Exception as error:
                    failures.append(error)
                    return
        workers = [threading.Thread(target=udp_echo), threading.Thread(target=tcp_echo)]
        for worker in workers:
            worker.start()
        transcript = bytearray()
        try:
            with tempfile.TemporaryDirectory(prefix='xv-socket-', dir=Path(args.image).parent) as temporary:
                image = Path(temporary)/'fs.img'
                shutil.copyfile(args.image, image)
                command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
                           '-m', '128M', '-smp', str(args.cpus), '-nographic', '-snapshot',
                           '-global', 'virtio-mmio.force-legacy=false',
                           '-drive', f'file={image},if=none,format=raw,id=x0',
                           '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
                           '-netdev', f'user,id=net0,hostfwd=tcp:127.0.0.1:{forward_port}-:2021',
                           '-device', 'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56']
                forward.close()
                for attempt in range(5):
                    process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                               stderr=subprocess.STDOUT, bufsize=0)
                    with selectors.DefaultSelector() as startup:
                        startup.register(process.stdout, selectors.EVENT_READ)
                        ready = startup.select(0.25)
                        prefix = os.read(process.stdout.fileno(), 65536) if ready else b''
                    transcript.extend(prefix)
                    if b'Could not set up host forwarding rule' not in prefix:
                        break
                    process.wait(timeout=2)
                    process.stdin.close()
                    process.stdout.close()
                    if attempt == 4:
                        raise RuntimeError('QEMU forwarding ports repeatedly occupied')
                    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as reservation:
                        reservation.bind(('127.0.0.1', 0))
                        forward_port = reservation.getsockname()[1]
                        command[command.index('-netdev')+1] = f'user,id=net0,hostfwd=tcp:127.0.0.1:{forward_port}-:2021'
                selector = selectors.DefaultSelector()
                selector.register(process.stdout, selectors.EVENT_READ)
                cursor = 0
                log = Path(args.image).parent/f'socket-{args.cpus}cpu.log'
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
                            log.write_bytes(transcript)
                            if b'panic:' in transcript or b'sockettest: FAIL:' in transcript:
                                raise RuntimeError(transcript.decode(errors='replace'))
                    raise TimeoutError(f'waiting for {token}: '+transcript.decode(errors='replace'))
                def inbound():
                    try:
                        with socket.create_connection(('127.0.0.1', forward_port), timeout=8) as connection:
                            connection.settimeout(10)
                            data = bytes((i*11+3)&255 for i in range(7000))
                            connection.sendall(data)
                            connection.shutdown(socket.SHUT_WR)
                            reply = bytearray()
                            while chunk := connection.recv(8192):
                                reply.extend(chunk)
                            assert reply == data, 'passive open content mismatch'
                    except Exception as error:
                        failures.append(error)
                try:
                    expect('e1000: ready')
                    expect('$ ')
                    start = cursor
                    process.stdin.write(b'netctl dhcp show eth0\n'); process.stdin.flush(); expect('$ ')
                    if b'DHCP OFF' not in transcript[start:cursor]:
                        process.stdin.write(b'netctl dhcp wait eth0\n'); process.stdin.flush()
                        expect('eth0 DHCP BOUND'); expect('$ ')
                    for _ in range(2):
                        process.stdin.write(f'sockettest {udp.getsockname()[1]} {tcp.getsockname()[1]} {refused_port}\n'.encode())
                        process.stdin.flush()
                        expect('sockettest: listening')
                        guest_peer = threading.Thread(target=inbound)
                        guest_peer.start()
                        expect('sockettest: ALL PASSED')
                        expect('$ ')
                        guest_peer.join(timeout=1)
                        assert not guest_peer.is_alive()
                        assert not failures, failures
                    print(f'socket network: {args.cpus} CPUs, active/passive TCP, UDP and loopback passed')
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                    selector.close()
                    log.write_bytes(transcript)
        finally:
            stop.set()
            for worker in workers:
                worker.join(timeout=11)


if __name__ == '__main__':
    main()
