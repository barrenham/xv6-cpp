#!/usr/bin/env python3
"""Exercise the HTTP library and curl CLI against controlled real TCP peers."""
from console_login import login_root
import argparse
import os
from pathlib import Path
import selectors
import re
import shutil
import socket
import socketserver
import subprocess
import tempfile
import threading
import struct
import time


def payload(size):
    return bytes((i*37+11)&255 for i in range(size))


def checksum(data):
    data += b'\0' * (len(data) % 2)
    value = sum(struct.unpack('!%dH' % (len(data)//2), data))
    while value >> 16:
        value = (value & 65535) + (value >> 16)
    return (~value) & 65535


class EchoPeer:
    """Controlled Ethernet peer: duplicates #1, drops #2, validates rejection at #3."""
    def __init__(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind(('127.0.0.1', 0)); self.socket.settimeout(.1)
        reserved = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        reserved.bind(('127.0.0.1', 0)); self.destination = reserved.getsockname(); reserved.close()
        self.stopped = threading.Event()
        self.requests = []
        self.worker = threading.Thread(target=self.work, daemon=True)
        self.worker.start()

    def work(self):
        mac = bytes.fromhex('52550a000302'); address = socket.inet_aton('10.0.3.2')
        while not self.stopped.is_set():
            try: frame = self.socket.recv(65536)
            except socket.timeout: continue
            except OSError: return
            if len(frame) >= 42 and frame[12:14] == b'\x08\x06' and frame[20:22] == b'\0\1' and frame[38:42] == address:
                arp = struct.pack('!HHBBH6s4s6s4s',1,0x800,6,4,2,mac,address,frame[22:28],frame[28:32])
                self.socket.sendto(frame[6:12]+mac+b'\x08\x06'+arp,self.destination)
            if len(frame) < 42 or frame[12:14] != b'\x08\0' or frame[23] != 1 or frame[30:34] != address: continue
            ihl = (frame[14]&15)*4; length = struct.unpack('!H',frame[16:18])[0]
            echo = bytearray(frame[14+ihl:14+length])
            if len(echo) < 8 or echo[0] != 8: continue
            sequence = struct.unpack('!H',echo[6:8])[0];self.requests.append(sequence)
            assert checksum(echo) == 0
            if sequence == 2: continue
            echo[0] = 0
            def reply(data):
                data[2:4] = b'\0\0';data[2:4] = struct.pack('!H',checksum(data))
                header = bytearray(struct.pack('!BBHHHBBH4s4s',0x45,0,20+len(data),0,0,64,1,0,address,frame[26:30]))
                header[10:12] = struct.pack('!H',checksum(header))
                self.socket.sendto(frame[6:12]+mac+b'\x08\0'+header+data,self.destination)
            if sequence == 3:
                wrong = bytearray(echo);wrong[4] ^= 1;reply(wrong)
                wrong = bytearray(echo);wrong[-1] ^= 1;reply(wrong)
            reply(bytearray(echo))
            if sequence == 1: reply(bytearray(echo))

    def close(self):
        self.stopped.set();self.worker.join(timeout=2);self.socket.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('qemu', 'kernel', 'image'):
        parser.add_argument('--'+name, required=True)
    parser.add_argument('--cpus', type=int, default=5)
    args = parser.parse_args()
    failures, requests = [], []

    class Handler(socketserver.BaseRequestHandler):
        def handle(self):
            try:
                self.request.settimeout(8)
                request = bytearray()
                while b'\r\n\r\n' not in request:
                    chunk = self.request.recv(2048)
                    if not chunk:
                        return
                    request.extend(chunk)
                    assert len(request) < 4096
                first, *headers = bytes(request).decode().split('\r\n')
                method, path, version = first.split(' ')
                fields = dict(line.split(': ', 1) for line in headers if line)
                assert method in ('GET', 'HEAD') and version == 'HTTP/1.1'
                assert fields['Host'] == f'10.0.2.2:{self.server.server_address[1]}'
                assert fields['Connection'] == 'close' and fields['Accept-Encoding'] == 'identity'
                requests.append((method, path))
                status, extra, body = '200 OK', b'', b''
                if path == '/fixed':
                    body = payload(24000)
                elif path == '/text':
                    body = b'curl-ready\n'
                elif path in ('/chunked', '/tinychunks'):
                    body = payload(7000 if path == '/chunked' else 24000)
                    chunks = []
                    step = 137
                    for begin in range(0, len(body), step):
                        part = body[begin:begin+step]
                        chunks.append(f'{len(part):x};test=yes\r\n'.encode()+part+b'\r\n')
                    self.send(b'HTTP/1.1 200 OK\r\ntRaNsFeR-EnCoDiNg: chunked\r\n\r\n'+b''.join(chunks)+b'0\r\nX-Checksum: valid\r\n\r\n')
                    return
                elif path == '/eof':
                    self.send(b'HTTP/1.0 200 OK\r\n\r\n'+payload(4000))
                    return
                elif path == '/informational':
                    self.send(b'HTTP/1.1 103 Early Hints\r\nLink: </x>\r\n\r\n')
                    body = payload(13)
                elif path == '/empty':
                    status = '204 No Content'
                elif path == '/notfound':
                    status, body = '404 Not Found', payload(3)
                elif path in ('/dir/start', '/absolute', '/network', '/query', '/loop', '/https'):
                    port = self.server.server_address[1]
                    location = {'/dir/start': '../final?x=1#fragment',
                                '/absolute': f'http://10.0.2.2:{port}/final',
                                '/network': f'//10.0.2.2:{port}/final',
                                '/query': '?go=1', '/loop': '/loop',
                                '/https': 'https://example.com/'}[path]
                    status, extra, body = '302 Found', f'Location: {location}\r\n'.encode(), b'skipped'
                elif path in ('/final', '/final?x=1', '/query?go=1'):
                    body = payload(17)
                elif path == '/conflict':
                    self.send(b'HTTP/1.1 200 OK\r\nContent-Length: 2\r\nContent-Length: 3\r\n\r\nabc')
                    return
                elif path == '/ambiguous':
                    self.send(b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Length: 3\r\n\r\n0\r\n\r\n')
                    return
                elif path == '/badheader':
                    self.send(b'HTTP/1.1 200 OK\r\nBad Header: x\r\n\r\n')
                    return
                elif path in ('/badchunk', '/badcrlf', '/chunkshort'):
                    data = {'/badchunk': b'z\r\n', '/badcrlf': b'1\r\nAZZ\r\n', '/chunkshort': b'5\r\nabc'}[path]
                    self.send(b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n'+data)
                    return
                elif path == '/short':
                    self.send(b'HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\nabc')
                    return
                elif path == '/encoding':
                    self.send(b'HTTP/1.1 200 OK\r\nContent-Encoding: gzip\r\nContent-Length: 0\r\n\r\n')
                    return
                elif path == '/slow':
                    time.sleep(1.5)
                    self.send(b'HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n')
                    return
                elif path == '/trickle':
                    self.send(b'HTTP/1.1 200 OK\r\nContent-Length: 1000\r\n\r\n')
                    for _ in range(100):
                        self.request.sendall(b'x')
                        time.sleep(.04)
                    return
                else:
                    raise AssertionError(f'unexpected request path {path}')
                head = f'HTTP/1.1 {status}\r\nContent-Length: {len(body)}\r\n'.encode()+extra+b'\r\n'
                self.send(head+(b'' if method == 'HEAD' else body))
                if path == '/empty' or method == 'HEAD':
                    # The client must complete using framing rather than waiting for EOF.
                    time.sleep(1)
            except (BrokenPipeError, ConnectionResetError):
                pass
            except Exception as error:
                failures.append(error)

        def send(self, data):
            for size in (1, 2, 7, 101):
                self.request.sendall(data[:size])
                data = data[size:]
                time.sleep(.002)
            if data:
                self.request.sendall(data)

    class Server(socketserver.ThreadingTCPServer):
        daemon_threads = True

    with Server(('127.0.0.1', 0), Handler) as server:
        worker = threading.Thread(target=server.serve_forever, daemon=True)
        worker.start()
        port = server.server_address[1]
        echo_peer = EchoPeer()
        transcript = bytearray()
        log = Path(args.image).parent/f'http-{args.cpus}cpu.log'
        try:
            with tempfile.TemporaryDirectory(prefix='xv-http-', dir=Path(args.image).parent) as temporary:
                image = Path(temporary)/'fs.img'
                shutil.copyfile(args.image, image)
                command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
                           '-m', '128M', '-smp', str(args.cpus), '-nographic', '-snapshot',
                           '-global', 'virtio-mmio.force-legacy=false',
                           '-drive', f'file={image},if=none,format=raw,id=x0',
                           '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
                           '-netdev', 'user,id=net0,dhcpstart=10.0.2.100',
                           '-device', 'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56',
                           '-netdev', f'socket,id=net1,udp=127.0.0.1:{echo_peer.socket.getsockname()[1]},localaddr=127.0.0.1:{echo_peer.destination[1]}',
                           '-device', 'e1000,netdev=net1,bus=pcie.0,addr=05.0,mac=52:54:00:12:34:57']
                process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
                selector = selectors.DefaultSelector()
                selector.register(process.stdout, selectors.EVENT_READ)
                cursor = 0
                def expect(token, timeout=30):
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
                        if b'panic:' in transcript or b'usertrap():' in transcript or b'httptest: FAIL:' in transcript or failures:
                            raise RuntimeError(str(failures)+transcript.decode(errors='replace'))
                    raise TimeoutError(f'waiting for {token}: '+transcript.decode(errors='replace'))
                def run(command):
                    process.stdin.write((command+'\n').encode())
                    process.stdin.flush()
                def output(command):
                    start = cursor
                    run(command)
                    expect('$ ')
                    text = transcript[start:cursor].decode(errors='replace')
                    # Console echoes the command; only subsequent bytes are output.
                    return text.split('\n', 1)[1]
                try:
                    login_root(expect, process)
                    run('netctl dhcp wait eth0')
                    expect('eth0 DHCP BOUND')
                    expect('$ ')
                    for _ in range(2):
                        run(f'httptest {port}')
                        expect('httptest: ALL PASSED', 45)
                        expect('$ ')
                    base = f'http://10.0.2.2:{port}'
                    assert 'curl-ready' in output(f'curl {base}/text')
                    text = output(f'curl -I {base}/text')
                    assert 'HTTP/1.1 200 OK' in text and 'curl-ready' not in text
                    text = output(f'curl -i {base}/text')
                    assert 'HTTP/1.1 200 OK' in text and 'curl-ready' in text
                    assert '(22)' in output(f'curl -f {base}/notfound')
                    assert '(47)' in output(f'curl -L {base}/loop')
                    assert '(28)' in output(f'curl -m 1 {base}/slow')
                    assert '(1)' in output('curl https://example.com/')
                    assert 'curl:' not in output(f'curl -o download {base}/fixed')
                    run('httptest --file download')
                    expect('httptest: FILE PASSED')
                    expect('$ ')
                    # Compare the same binary chunk stream with and without output buffering.
                    counts = []
                    for flag in ('', '-N'):
                        text = output(f'curl {flag} -o download -w "%{{output_writes}} %{{size_download}}" {base}/tinychunks')
                        stats = re.search(r'(\d+) 24000', text)
                        assert stats, text
                        counts.append(int(stats[1]))
                        run('httptest --file download');expect('httptest: FILE PASSED');expect('$ ')
                    assert counts[0] <= 4 and counts[1] >= 175, counts
                    text = output(f'curl -o download -w "%{{http_code}} %{{time_total}}" {base}/text')
                    assert re.search(r'200 [0-9]+\.[0-9]{6}',text),text
                    text = output(f'curl -s -m .1 -o download {base}/slow')
                    assert 'curl:' not in text, text
                    assert '(28)' in output(f'curl -sS -m .1 {base}/slow')
                    assert 'curl-ready' in output(f'curl --connect-timeout .5 {base}/text')
                    output(f'curl -o download {base}/fixed')
                    assert '(1)' in output('curl -o download https://example.com/')
                    run('httptest --file download');expect('httptest: FILE PASSED');expect('$ ')
                    assert 'curl:' not in output(f'curl -sS -o download {base}/text')
                    assert 'usage:' in output(f'curl -w "%{{unknown}}" {base}/text')
                    assert 'usage:' in output(f'curl -m 1.0001 {base}/text')
                    # Real ICMP sockets through syscall/loopback, signal handling, and shell integration.
                    for size in (0, 56, 1400):
                        text=output(f'ping -c 3 -i .01 -W .2 -s {size} 127.0.0.1')
                        assert '3 packets transmitted, 3 received' in text and '0% packet loss' in text, text
                        assert 'rtt min/avg/max =' in text, text
                    text=output('ping -q -c 2 -i .01 127.0.0.1')
                    assert '2 packets transmitted, 2 received' in text and 'icmp_seq=' not in text, text
                    output('netctl dhcp stop eth1')
                    output('netctl addr add 10.0.3.15/24 dev eth1')
                    text=output('ping -c 3 -i .01 -W .2 10.0.3.2')
                    assert '3 packets transmitted, 2 received, 1 duplicates, 33% packet loss' in text, text
                    assert echo_peer.requests == [1,2,3],echo_peer.requests
                    text=output('curl --connect-timeout .1 -w "%{time_total}" http://10.0.3.2/')
                    assert '(28)' in text,text
                    elapsed=re.search(r'([0-9]+\.[0-9]{6})',text)
                    assert elapsed and .09 <= float(elapsed[1]) < 2,text
                    # Drop inbound echo packets deterministically, independent of external network access.
                    output('iptables -I INPUT 1 -p icmp -j DROP')
                    text=output('ping -c 2 -i .01 -W .1 127.0.0.1')
                    assert '2 packets transmitted, 0 received' in text and '100% packet loss' in text, text
                    output('iptables -F INPUT')
                    run('ping -i .05 127.0.0.1');expect('icmp_seq=2')
                    process.stdin.write(b'\x03');process.stdin.flush()
                    expect('ping statistics');expect('$ ')
                    assert 'usage:' in output('ping -c 0 127.0.0.1')
                    assert '\x1b[H\x1b[2J\x1b[3J' in output('clear')
                    text=output('clear -x')
                    assert '\x1b[H\x1b[2J' in text and '\x1b[3J' not in text, text
                    assert 'usage:' in output('clear bad')
                    print(f'curl output calls buffered/unbuffered: {counts}; ping loopback/loss/interrupt and clear passed')
                    assert not failures, failures
                    assert ('HEAD', '/text') in requests and ('GET', '/final?x=1') in requests
                    print(f'HTTP: {args.cpus} CPUs, HTTP framing, binary streaming, DNS, redirects, deadline and curl CLI passed')
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
            echo_peer.close()
            log.write_bytes(transcript)
            server.shutdown()
            worker.join(timeout=2)


if __name__ == '__main__':
    main()
