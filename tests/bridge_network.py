#!/usr/bin/env python3
"""Bridge real E1000 ports over independent raw Ethernet UDP backends."""
from console_login import login_root
import argparse
import contextlib
import os
from pathlib import Path
import selectors
import re
import shutil
import socket
import struct
import subprocess
import tempfile
import time


def checksum(data):
    if len(data) % 2:
        data += b'\0'
    total = sum(struct.unpack('!%dH' % (len(data)//2), data))
    while total >> 16:
        total = (total & 65535)+(total >> 16)
    return (~total) & 65535


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', required=True)
    parser.add_argument('--image', required=True)
    parser.add_argument('--cpus', type=int, default=5)
    args = parser.parse_args()
    a, b, c = [bytes.fromhex('0200000000'+suffix) for suffix in ('a1', 'b2', 'c3')]
    bridge_mac = None
    with contextlib.ExitStack() as stack:
        peers = [stack.enter_context(socket.socket(socket.AF_INET, socket.SOCK_DGRAM)) for _ in range(3)]
        reserved = [stack.enter_context(socket.socket(socket.AF_INET, socket.SOCK_DGRAM)) for _ in range(3)]
        for sock in peers+reserved:
            sock.bind(('127.0.0.1', 0))
        addresses = [sock.getsockname() for sock in reserved]
        for sock in peers:
            sock.setblocking(False)
        temporary = stack.enter_context(tempfile.TemporaryDirectory(prefix='xv-bridge-', dir=Path(args.image).parent))
        image = Path(temporary)/'fs.img'
        shutil.copyfile(args.image, image)
        command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
                   '-m', '128M', '-smp', str(args.cpus), '-nographic', '-snapshot',
                   '-global', 'virtio-mmio.force-legacy=false',
                   '-drive', f'file={image},if=none,format=raw,id=x0',
                   '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
        for i, peer in enumerate(peers):
            command += ['-netdev', f'socket,id=n{i},udp=127.0.0.1:{peer.getsockname()[1]},localaddr=127.0.0.1:{addresses[i][1]}',
                        '-device', f'e1000,netdev=n{i},bus=pcie.0,addr={1+i*4:02x}.0,mac=52:54:00:12:34:{0x56+i:02x}']
        for sock in reserved:
            sock.close()
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
        transcript = bytearray()
        console = selectors.DefaultSelector()
        console.register(process.stdout, selectors.EVENT_READ)
        cursor = 0
        def expect(token, timeout=20):
            nonlocal cursor
            deadline = time.monotonic()+timeout
            while time.monotonic() < deadline:
                position = transcript.find(token.encode(), cursor)
                if position >= 0:
                    cursor = position+len(token)
                    return
                if process.poll() is not None:
                    raise RuntimeError('QEMU exited: '+transcript.decode(errors='replace'))
                for key, _ in console.select(0.1):
                    transcript.extend(os.read(key.fd, 65536))
                if b'panic:' in transcript or b'bridgetest: FAIL:' in transcript:
                    raise RuntimeError(transcript.decode(errors='replace'))
            raise TimeoutError(f'waiting for {token}: '+transcript.decode(errors='replace'))
        def run(text, token=None):
            start = len(transcript)
            process.stdin.write((text+'\n').encode()); process.stdin.flush()
            if token:
                expect(token)
            expect('$ ')
            if b'netctl: ' in transcript[start:]:
                raise RuntimeError(transcript[start:].decode(errors='replace'))
            return bytes(transcript[start:])
        def collect(duration=0.2):
            result = [[] for _ in peers]
            deadline = time.monotonic()+duration
            while time.monotonic() < deadline:
                for i, peer in enumerate(peers):
                    try:
                        while True:
                            result[i].append(peer.recv(4096))
                    except BlockingIOError:
                        pass
                time.sleep(0.005)
            return result
        def frame(source, destination, marker, kind=0x88b5):
            return (destination+source+struct.pack('!H', kind)+marker).ljust(60, b'\0')
        def forward(port, packet, outputs):
            collect(0.03)
            peers[port].sendto(packet, addresses[port])
            received = collect()
            # QEMU E1000 defers RX for up to one second after an RCTL write.
            deadline = time.monotonic()+2
            while any(not received[i] for i in outputs) and time.monotonic()<deadline:
                more = collect(0.05)
                for i in range(len(peers)):
                    received[i].extend(more[i])
            more = collect(0.1)
            for i in range(len(peers)):
                received[i].extend(more[i])
            for i, frames in enumerate(received):
                if frames != ([packet] if i in outputs else []):
                    run('netctl bridge fdb br0')
                    run('netctl stats')
                    raise AssertionError((port, i, outputs, frames))
        try:
            expect('e1000: ready'); login_root(expect, process)
            for i in range(3):
                status = run(f'netctl dhcp show eth{i}')
                run(f'netctl dhcp stop eth{i}')
                if b'DHCP OFF' in status:
                    run(f'netctl route del default dev eth{i} via 10.0.{2+i}.2 metric {i*10}')
                    run(f'netctl addr del 10.0.{2+i}.15/24 dev eth{i}')
            for round_number in range(2):
                run('netctl bridge create br0')
                link = run('netctl link show br0')
                bridge_mac = bytes.fromhex(re.search(rb'mac ([0-9a-f:]{17})', link)[1].decode().replace(':', ''))
                for i in range(3):
                    run(f'netctl bridge addport br0 eth{i}')
                run('netctl link set br0 up')
                run('netctl bridge show br0', 'br0 id 16 UP ports: eth0 eth1 eth2')
                run('netctl link show eth0', 'master 16')
                forward(0, frame(a, b, b'unknown'), {1, 2})
                forward(1, frame(b, a, b'learned'), {0})
                forward(0, frame(a, b, b'directed'), {1})
                forward(0, frame(a, bytes([255])*6, b'broadcast'), {1, 2})
                forward(0, frame(a, a, b'filtered'), set())
                forward(0, frame(a, b, b'vlan', 0x8100), set())
                run('netctl bridge fdb br0', '02:00:00:00:00:a1 dev eth0')
                forward(2, frame(b, a, b'move'), {0})
                forward(0, frame(a, b, b'moved'), {2})
                run('netctl bridge flush br0')
                forward(0, frame(a, b, b'flush'), {1, 2})
                run('netctl addr add 10.20.0.1/24 dev br0')
                src_ip, dst_ip = socket.inet_aton('10.20.0.2'), socket.inet_aton('10.20.0.1')
                arp = struct.pack('!HHBBH6s4s6s4s', 1, 0x800, 6, 4, 1, a, src_ip, b'\0'*6, dst_ip)
                peers[0].sendto(frame(a, b'\xff'*6, arp, 0x806), addresses[0])
                packets = collect(0.4)
                replies = [p for p in packets[0] if p[12:14] == b'\x08\x06' and p[20:22] == b'\0\2']
                assert replies and replies[0][:12] == a+bridge_mac and replies[0][28:32] == dst_ip, packets
                process.stdin.write(b'bridgetest\n'); process.stdin.flush(); expect('bridgetest: ready')
                payload = b'bridge-host'
                udp = struct.pack('!HHHH', 40000, 2033, 8+len(payload), 0)+payload
                header = struct.pack('!BBHHHBBH4s4s', 0x45, 0, 20+len(udp), round_number, 0, 64, 17, 0, src_ip, dst_ip)
                header = header[:10]+struct.pack('!H', checksum(header))+header[12:]
                collect(0.03)
                peers[0].sendto(frame(a, bridge_mac, header+udp, 0x800), addresses[0])
                replies = collect(0.6)
                matches = [p for p in replies[0] if p[12:14] == b'\x08\x00' and p[23] == 17]
                assert matches, replies
                reply = matches[0]
                assert reply[:12] == a+bridge_mac and reply[26:34] == dst_ip+src_ip
                assert checksum(reply[14:34]) == 0 and reply[34:38] == struct.pack('!HH', 2033, 40000)
                assert reply[42:53] == payload and not replies[1] and not replies[2], replies
                expect('bridgetest: ALL PASSED'); expect('$ ')
                run('netctl addr del 10.20.0.1/24 dev br0')
                run('netctl link set br0 down')
                for i in range(3):
                    run(f'netctl bridge delport br0 eth{i}')
                run('netctl bridge delete br0')
            print(f'bridge: {args.cpus} CPUs, three real NICs, learning/flooding/local ARP/UDP and recreation passed')
        finally:
            process.terminate()
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                process.kill(); process.wait()
            (Path(args.image).parent/f'bridge-{args.cpus}cpu.log').write_bytes(transcript)
            console.close()


if __name__ == '__main__':
    main()
