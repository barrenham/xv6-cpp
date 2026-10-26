#!/usr/bin/env python3
"""Exercise the HTTP library and curl CLI against controlled real TCP peers."""
import argparse
import os
from pathlib import Path
import selectors
import shutil
import socket
import socketserver
import subprocess
import tempfile
import threading
import time


def payload(size):
    return bytes((i*37+11)&255 for i in range(size))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('qemu', 'kernel', 'image'):
        parser.add_argument('--'+name, required=True)
    parser.add_argument('--cpus', type=int, default=3)
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
                elif path == '/chunked':
                    body = payload(7000)
                    chunks = []
                    for begin in range(0, len(body), 137):
                        part = body[begin:begin+137]
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
                           '-device', 'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56']
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
                    expect('$ ')
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
            log.write_bytes(transcript)
            server.shutdown()
            worker.join(timeout=2)


if __name__ == '__main__':
    main()
