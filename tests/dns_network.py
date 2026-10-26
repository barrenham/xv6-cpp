#!/usr/bin/env python3
"""DNS resolver against deterministic host peers and QEMU's DHCP-selected DNS proxy."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import socket
import struct
import subprocess
import tempfile
import threading
import time


def wire(name):
    return b''.join(bytes([len(label)]) + label.encode() for label in name.split('.')) + b'\0'


def record(name, kind, data, ttl=90):
    return name + struct.pack('!HHIH', kind, 1, ttl, len(data)) + data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for flag in ('qemu', 'kernel', 'image'):
        parser.add_argument('--' + flag, required=True)
    parser.add_argument('--cpus', type=int, default=3)
    parser.add_argument('--public', action='store_true', help='also resolve example.com through the host upstream DNS')
    args = parser.parse_args()
    stop = threading.Event()
    failures = []
    seen = {}
    transports = set()
    udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    udp.bind(('127.0.0.1', 0))
    port = udp.getsockname()[1]
    tcp.bind(('127.0.0.1', port))
    tcp.listen(4)
    udp.settimeout(.1)
    tcp.settimeout(.1)

    def response(query, stream=False):
        labels, cursor = [], 12
        while query[cursor]:
            length = query[cursor]
            labels.append(query[cursor+1:cursor+1+length].decode())
            cursor += length+1
        cursor += 5
        name = '.'.join(labels)
        question = query[12:cursor]
        seen[name] = seen.get(name, 0)+1
        transports.add((name, stream))
        flags, records = 0x8180, []
        a = lambda owner, ip, ttl=90: record(owner, 1, socket.inet_aton(ip), ttl)
        owner = b'\xc0\x0c'
        if name == 'silent.test' or (name == 'retry.test' and seen[name] % 2):
            return None
        if name == 'missing.test':
            flags = 0x8183
        elif name == 'fail.test':
            flags = 0x8182
        elif name == 'mixed.test':
            records = [a(owner, '192.0.2.1'), a(owner, '192.0.2.2'), a(owner, '192.0.2.1')]
        elif name == 'alias.test':
            # Target precedes the CNAME; RDATA uses a pointer to that target owner.
            records = [a(wire('target.test'), '192.0.2.9'), record(owner, 5, struct.pack('!H', 0xc000 | cursor), 20)]
        elif name == 'follow.test':
            records = [record(owner, 5, wire('target.test'), 10)]
        elif name == 'loop.test':
            records = [record(owner, 5, wire('other.test'), 10)]
        elif name == 'other.test':
            records = [record(owner, 5, wire('loop.test'), 10)]
        elif name == 'empty.test':
            records = [a(wire('unrelated.test'), '192.0.2.99')]
        elif name == 'tcp.test' and not stream:
            flags = 0x8380
        else:
            records = [a(owner, '192.0.2.10' if stream else '192.0.2.9')]
        return struct.pack('!HHHHHH', struct.unpack('!H', query[:2])[0], flags, 1, len(records), 0, 0) + question + b''.join(records)

    def udp_server():
        try:
            while not stop.is_set():
                try:
                    query, peer = udp.recvfrom(512)
                except socket.timeout:
                    continue
                reply = response(query)
                if reply is None:
                    continue
                if wire('forged.test') in query:
                    wrong_id = bytes([reply[0] ^ 1]) + reply[1:]
                    udp.sendto(wrong_id, peer)
                    bad_question = bytearray(reply)
                    bad_question[13] = ord('z')
                    udp.sendto(bad_question, peer)
                    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as outsider:
                        outsider.sendto(reply[:-4]+socket.inet_aton('192.0.2.99'), peer)
                    bad_pointer = reply[:len(query)] + b'\xc0\xff' + reply[len(query)+2:]
                    udp.sendto(bad_pointer, peer)
                    oversized = bytearray(reply[:-4] + socket.inet_aton('192.0.2.99'))
                    oversized[11] = 1
                    padding = 512 - len(oversized) - 11
                    oversized += record(b'\0', 65000, bytes(padding))
                    assert len(oversized) == 512
                    udp.sendto(oversized+b'X', peer)
                udp.sendto(reply, peer)
        except Exception as error:
            failures.append(error)

    def exact(connection, count):
        data = bytearray()
        while len(data) < count:
            chunk = connection.recv(count-len(data))
            if not chunk:
                raise RuntimeError('short TCP DNS query')
            data.extend(chunk)
        return bytes(data)

    def tcp_server():
        try:
            while not stop.is_set():
                try:
                    connection, _ = tcp.accept()
                except socket.timeout:
                    continue
                with connection:
                    connection.settimeout(5)
                    length = struct.unpack('!H', exact(connection, 2))[0]
                    reply = response(exact(connection, length), True)
                    frame = struct.pack('!H', len(reply)) + reply
                    connection.sendall(frame[:1])
                    time.sleep(.02)
                    connection.sendall(frame[1:9])
                    time.sleep(.02)
                    connection.sendall(frame[9:])
        except Exception as error:
            failures.append(error)

    workers = [threading.Thread(target=udp_server), threading.Thread(target=tcp_server)]
    for worker in workers:
        worker.start()
    transcript = bytearray()
    log = Path(args.image).parent / f'dns-{args.cpus}cpu.log'
    try:
        with tempfile.TemporaryDirectory(prefix='xv-dns-', dir=Path(args.image).parent) as temporary:
            image = Path(temporary) / 'fs.img'
            shutil.copyfile(args.image, image)
            command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
                       '-m', '128M', '-smp', str(args.cpus), '-nographic', '-snapshot',
                       '-global', 'virtio-mmio.force-legacy=false',
                       '-drive', f'file={image},if=none,format=raw,id=x0',
                       '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
                       '-netdev', 'user,id=net0,dhcpstart=10.0.2.100',
                       '-device', 'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56']
            process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
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
                    for key, _ in selector.select(.1):
                        transcript.extend(os.read(key.fd, 65536))
                    if b'panic:' in transcript or b'dnstest: FAIL:' in transcript:
                        raise RuntimeError(transcript.decode(errors='replace'))
                raise TimeoutError(f'waiting for {token}: '+transcript.decode(errors='replace'))
            def run(command):
                process.stdin.write((command+'\n').encode())
                process.stdin.flush()
            try:
                expect('$ ')
                run('netctl dhcp wait eth0')
                expect('eth0 DHCP BOUND')
                expect('$ ')
                for _ in range(2):
                    run(f'dnstest {port}')
                    expect('dnstest: ALL PASSED')
                    expect('$ ')
                run('nslookup localhost')
                expect('Server: 10.0.2.3')
                expect('Address: ')
                expect('$ ')
                run('nslookup 192.0.2.7')
                expect('Address: 192.0.2.7')
                expect('$ ')
                if args.public:
                    run('nslookup example.com')
                    expect('Server: 10.0.2.3')
                    expect('Name: example.com')
                    expect('Address: ')
                    expect('$ ')
                run('netctl dhcp stop eth0')
                expect('$ ')
                run('nslookup localhost')
                expect('no DHCP DNS server; specify a server address')
                expect('$ ')
                run('nslookup 192.0.2.7')
                expect('Address: 192.0.2.7')
                expect('$ ')
                for command in ('netctl bridge create br0', 'netctl bridge addport br0 eth0',
                                'netctl link set br0 up', 'netctl dhcp start br0'):
                    run(command)
                    expect('$ ')
                run('netctl dhcp wait br0')
                expect('br0 DHCP BOUND')
                expect('$ ')
                run('nslookup localhost')
                expect('Server: 10.0.2.3')
                expect('Address: ')
                expect('$ ')
                assert not failures, failures
                assert ('tcp.test', True) in transports
                assert seen.get('retry.test', 0) >= 4
                print(f'DNS: {args.cpus} CPUs, UDP/TCP, validation, retries, CNAME and DHCP/QEMU DNS proxy passed')
            finally:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                selector.close()
                process.stdin.close()
                process.stdout.close()
    finally:
        log.write_bytes(transcript)
        stop.set()
        for worker in workers:
            worker.join(timeout=6)
        udp.close()
        tcp.close()


if __name__ == '__main__':
    main()
