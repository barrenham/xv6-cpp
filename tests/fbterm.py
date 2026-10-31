#!/usr/bin/env python3
"""Exercise fbterm on an isolated root disk and inspect real GPU pixels via QMP."""
import argparse
import json
import os
from pathlib import Path
import re
import selectors
import shutil
import socket
import subprocess
import struct
import sys
import tempfile
import time
from console_login import login_root

parser = argparse.ArgumentParser()
parser.add_argument('--qemu', required=True)
parser.add_argument('--kernel', required=True)
parser.add_argument('--image', required=True)
parser.add_argument('--cpus', type=int, default=5)
parser.add_argument('--without-gpu', action='store_true')
parser.add_argument('--keyboard', action='store_true')
parser.add_argument('--width', type=int, default=1280)
parser.add_argument('--height', type=int, default=800)
args = parser.parse_args()
output = Path(args.image).parent
transcript = bytearray()
cursor = 0
# The checked-in bitmap is the font asset, used to decode the actual scanout.
font_source = (Path(__file__).resolve().parents[1] / 'user/apps/fbterm/font.h').read_text()
font_source = re.sub(r'//[^\n]*', '', font_source)
cell_width = int(re.search(r'font_width\s*=\s*(\d+)', font_source)[1])
cell_height = int(re.search(r'font_height\s*=\s*(\d+)', font_source)[1])
columns, rows = args.width // cell_width, args.height // cell_height
bitmaps = re.findall(r'\{([\d,\s]+)\}', font_source)
font = [tuple(int(value) for value in row.split(',') if value.strip()) for row in bitmaps]
assert len(font) == 95
assert all(len(glyph) == cell_width * cell_height for glyph in font)
lookup = {}
for index, glyph in enumerate(font):
    mask = tuple(sum((glyph[y*cell_width+x] > 0) << (cell_width-1-x)
                     for x in range(cell_width)) for y in range(cell_height))
    assert mask not in lookup, (index, lookup.get(mask))
    lookup[mask] = chr(32 + index)

with tempfile.TemporaryDirectory(prefix='xv-fbterm-', dir=output) as directory:
    directory = Path(directory)
    disk = directory / 'fs.img'
    shutil.copyfile(args.image, disk)
    # Install a finite burst fixture only into the test's private disk.
    sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
    from create_rootfs import Image, FILE
    burst = ''.join(f'row-{i:05d}: fast terminal output 0123456789\n'
                    for i in range(2000)) + 'BURST_END\n'
    fixture = Image(disk)
    number = fixture.create(fixture.root, b'burst.txt', FILE)
    fixture.append(number, burst.encode())
    disk.write_bytes(fixture.data)
    del fixture
    monitor = directory / 'qmp'
    command = [args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', args.kernel,
               '-m', '128M', '-smp', str(args.cpus), '-display', 'none', '-serial', 'stdio',
               '-qmp', f'unix:{monitor},server=on,wait=off', '-global', 'virtio-mmio.force-legacy=false',
               '-drive', f'file={disk},if=none,format=raw,id=x0',
               '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
    if not args.without_gpu:
        command += ['-device', f'virtio-gpu-device,bus=virtio-mmio-bus.3,xres={args.width},yres={args.height}']
    if args.keyboard:
        command += ['-device', 'virtio-keyboard-device,bus=virtio-mmio-bus.4']
        with socket.socket() as reservation:
            reservation.bind(('127.0.0.1', 0))
            vnc_port = reservation.getsockname()[1]
        command += ['-vnc', f'127.0.0.1:{vnc_port - 5900}']
    guest = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, bufsize=0)
    selector = selectors.DefaultSelector()
    selector.register(guest.stdout, selectors.EVENT_READ)

    def send(data):
        guest.stdin.write(data.encode() if isinstance(data, str) else data)
        guest.stdin.flush()

    def expect(token, timeout=45):
        global cursor
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
        raise TimeoutError(f'waiting for {token!r}')

    def screenshot(name):
        # Let the bounded display coalescer publish its final pending update.
        time.sleep(.4)
        path = directory / 'screen.ppm'
        with socket.socket(socket.AF_UNIX) as connection:
            connection.settimeout(10)
            connection.connect(str(monitor))
            stream = connection.makefile('rwb', buffering=0)
            json.loads(stream.readline())
            for message in ({'execute': 'qmp_capabilities'},
                            {'execute': 'screendump', 'arguments': {'filename': str(path)}}):
                stream.write(json.dumps(message).encode() + b'\n')
                while True:
                    reply = json.loads(stream.readline())
                    assert 'error' not in reply, reply
                    if 'return' in reply:
                        break
        with path.open('rb') as image:
            assert image.readline().strip() == b'P6'
            assert image.readline().strip() == f'{args.width} {args.height}'.encode()
            assert image.readline().strip() == b'255'
            pixels = image.read()
        assert len(pixels) == args.width * args.height * 3
        shutil.copyfile(path, output / f'fbterm-{args.cpus}cpu-{name}.ppm')
        lines = []
        for row in range(rows):
            line = ''
            for column in range(columns):
                glyph = []
                for y in range(cell_height):
                    bits = 0
                    for x in range(cell_width):
                        start = ((row * cell_height + y) * args.width + column * cell_width + x) * 3
                        bits = (bits << 1) | (pixels[start:start+3] != b'\x10\x18\x20')
                    glyph.append(bits)
                decoded = lookup.get(tuple(glyph))
                if decoded is None and glyph[-2:] == [(1 << cell_width)-1] * 2:
                    # Underline cursor over a blank or a glyph with empty bottom rows.
                    decoded = lookup.get(tuple(glyph[:-2] + [0, 0]))
                line += decoded if decoded is not None else '?'
            lines.append(line)
        return lines, pixels

    def vnc_connect():
        remote = socket.create_connection(('127.0.0.1', vnc_port), timeout=10)
        def receive(count):
            data = b''
            while len(data) < count:
                part = remote.recv(count - len(data))
                assert part, 'VNC disconnected'
                data += part
            return data
        assert receive(12) == b'RFB 003.008\n'
        remote.sendall(b'RFB 003.008\n')
        assert 1 in receive(receive(1)[0])
        remote.sendall(b'\x01')
        assert receive(4) == b'\0\0\0\0'
        remote.sendall(b'\x01')
        header = receive(24)
        receive(struct.unpack('!I', header[20:])[0])
        return remote

    def key(remote, symbol, down):
        remote.sendall(struct.pack('!BBHI', 4, down, 0, symbol))
        time.sleep(.015)

    def press(remote, symbol):
        key(remote, symbol, 1)
        key(remote, symbol, 0)

    def type_vnc(remote, text):
        for value in text:
            shifted = {'_': '-', '+': '=', ':': ';', '"': "'", '?': '/',
                       '!': '1', '@': '2', '#': '3', '$': '4', '%': '5',
                       '^': '6', '&': '7', '*': '8', '(': '9', ')': '0',
                       '{': '[', '}': ']', '|': '\\', '<': ',', '>': '.', '~': '`'}
            needs_shift = value.isupper() or value in shifted
            if needs_shift: key(remote, 0xffe1, 1)
            press(remote, 0xff0d if value == '\n' else ord(value))
            if needs_shift: key(remote, 0xffe1, 0)

    def run(command, marker, timeout=45):
        send(command + '\n')
        expect(marker, timeout)
        expect('root:/$ ')

    try:
        login_root(expect, guest)
        if args.without_gpu:
            run('fbterm', 'fbterm: supported BGRX framebuffer required')
            run('echo ABSENT_OK', 'ABSENT_OK\n')
        else:
            run('fbterm --demo', 'fbterm: demo displayed')
            lines, pixels = screenshot('demo')
            assert lines[0].startswith('scroll 06:'), lines
            assert lines[rows-2].startswith(f'scroll {rows+4:02d}:'), lines
            assert lines[rows-1].startswith('fbterm demo complete'), lines
            # Green SGR footer, not just text-shaped non-background pixels.
            footer = pixels[(rows-1)*cell_height*args.width*3:rows*cell_height*args.width*3]
            assert any(footer[i:i+3] == b'\x55\xaa\x55' for i in range(0,len(footer),3))
            send('fbterm\n')
            expect(f'fbterm: connected ({columns}x{rows})')
            expect('root:/$ ')
            if args.keyboard:
                with vnc_connect() as remote:
                    type_vnc(remote, 'echo vnc_direct\n')
                    expect('vnc_direct\r\n'); expect('root:/$ ')
                    lines, _ = screenshot('vnc')
                    assert any(line.startswith('vnc_direct') for line in lines), lines
                    type_vnc(remote, 'echo shift_')
                    key(remote, 0xffe1, 1); press(remote, ord('A')); key(remote, 0xffe1, 0)
                    press(remote, 0xffe5); press(remote, ord('B')); press(remote, 0xffe5)
                    type_vnc(remote, '\n')
                    expect('shift_AB\r\n'); expect('root:/$ ')
                    type_vnc(remote, 'echo vnc_x')
                    press(remote, 0xff08)
                    type_vnc(remote, 'ok\n')
                    expect('vnc_ok\r\n'); expect('root:/$ ')
                    # Up arrow recalls the last command; Enter executes it again.
                    press(remote, 0xff52); press(remote, 0xff0d)
                    expect('vnc_ok\r\n'); expect('root:/$ ')
                    type_vnc(remote, 'cat\n')
                    expect('cat\r\n')
                    type_vnc(remote, 'vnc_cat\n')
                    expect('vnc_cat\r\n'); expect('vnc_cat\r\n')
                    key(remote, 0xffe3, 1); press(remote, ord('c')); key(remote, 0xffe3, 0)
                    expect('root:/$ ')
            run('ls /bin/fbterm', 'fbterm')
            lines, _ = screenshot('ls')
            assert any('fbterm' in line for line in lines), lines
            run('ps', 'PID PPID PGID SID STATE KiB NAME')
            lines, _ = screenshot('ps')
            assert any('PID PPID PGID SID STATE KiB NAME' in line for line in lines), lines
            run('free', 'Mem:')
            lines, _ = screenshot('free')
            assert any('Mem:' in line for line in lines), lines
            # More than one proxy buffer: verify every byte survives backpressure.
            payload = (Path(__file__).resolve().parents[1] / 'README.xv6').read_text()
            assert len(payload.encode()) > 1024
            run('cat /usr/share/doc/xv6/README', payload.replace('\n', '\r\n'))
            # Exercise clear plus editor redraw and backspace through the PTY.
            send('clear\n')
            expect('\x1b[3J')
            expect('root:/$ ')
            run('echo EDIT_X\x7fOK', 'EDIT_OK\r\n')
            lines, _ = screenshot('edited')
            assert any(line.startswith('EDIT_OK') for line in lines), lines
            assert not any('Mem:' in line or 'PID PPID' in line for line in lines), lines
            run('stty -a', 'icanon echo')
            send('clear\n'); expect('\x1b[3J'); expect('root:/$ ')
            long_text = 'a' * (columns + 29)
            command = 'echo ' + long_text + 'TAIL'
            for start in range(0,len(command),16):
                send(command[start:start+16]); time.sleep(.02)
            expect('TAIL')
            lines, _ = screenshot('long-input')
            joined = ''.join(line.rstrip() for line in lines[:3])
            assert joined.startswith('root:/$ echo ' + long_text + 'TAIL'), lines
            assert sum(line.startswith('root:/$ ') for line in lines) == 1, lines
            send(b'\x7f')
            expect('\x1b[J')
            # Move left, insert and restore the end across wrapped rows.
            send(b'\x01\x05')
            expect('\x1b[J')
            send('b\n')
            expect(long_text + 'TAIb\r\n'); expect('root:/$ ')
            lines, _ = screenshot('long-edit')
            assert not any(line.strip() == 'a' for line in lines), lines
            run('cat /burst.txt', burst.replace('\n', '\r\n'), timeout=120)
            # Serial completion can precede the coalesced final GPU frame.
            for attempt in range(10):
                lines, _ = screenshot('burst-tail')
                if any(line.startswith('BURST_END') for line in lines):
                    break
            assert any(line.startswith('BURST_END') for line in lines), lines
            assert any(line.startswith('row-01999:') for line in lines), lines
            # Nested proxies own only their own escape. Each child inherits the
            # graphical output conversion, even while its parent forwards raw.
            send('clear\n'); expect('\x1b[3J'); expect('root:/$ ')
            nested_start = len(transcript)
            run('pty', 'pty: connected; Ctrl-] exits\r\n')
            run('echo NESTED_ONE', 'NESTED_ONE\r\n')
            run('pty', 'pty: connected; Ctrl-] exits\r\n')
            run('echo NESTED_TWO', 'NESTED_TWO\r\n')
            for attempt in range(10):
                lines, _ = screenshot('nested-pty')
                if any(line.startswith('NESTED_TWO') for line in lines):
                    break
            assert any(line.startswith('NESTED_ONE') for line in lines), lines
            assert any(line.startswith('NESTED_TWO') for line in lines), lines
            assert not any(line.lstrip().startswith('root:/$ ') and
                           not line.startswith('root:/$ ') for line in lines), lines
            send(b'\x1d')
            expect('pty: disconnected\r\n'); expect('root:/$ ')
            run('echo STILL_ONE', 'STILL_ONE\r\n')
            if args.keyboard:
                with vnc_connect() as remote:
                    key(remote, 0xffe3, 1)
                    press(remote, ord(']'))
                    key(remote, 0xffe3, 0)
            else:
                send(b'\x1d')
            expect('pty: disconnected\r\n'); expect('root:/$ ')
            assert transcript[nested_start:].count(b'pty: disconnected') == 2
            assert b'fbterm: disconnected' not in transcript[nested_start:]
            run('echo FBTERM_ALIVE', 'FBTERM_ALIVE\r\n')
            run('ptytest checkflags', 'ptytest: terminal flags restored')
            # Signal bytes must reach the inner job, not kill the proxy.
            send('cat\n')
            expect('cat\r\n')
            send('PTY_INPUT\n')
            expect('PTY_INPUT\r\n')
            expect('PTY_INPUT\r\n') # cat consumed the line, beyond the TTY echo.
            send(b'\x03')
            expect('root:/$ ')
            send(b'\x04')
            expect('fbterm: disconnected')
            expect('root:/$ ')
            run('echo RESTORED', 'RESTORED\n')
            run('ptytest checkflags', 'ptytest: terminal flags restored')
            # Explicit detach must also restore raw mode and shared fd flags.
            send('fbterm\n')
            expect(f'fbterm: connected ({columns}x{rows})')
            expect('root:/$ ')
            send('jobtest raw\n')
            expect('jobtest: raw ready')
            send(b'\x1d')
            expect('fbterm: disconnected')
            expect('root:/$ ')
            run('echo DETACHED', 'DETACHED\n')
            run('ptytest checkflags', 'ptytest: terminal flags restored')
            # Mirror-off still accepts serial input and renders PTY output.
            send('fbterm --no-serial-mirror\n')
            expect(f'fbterm: connected ({columns}x{rows})')
            for attempt in range(10):
                lines, _ = screenshot('no-serial-mirror-ready')
                if any(line.startswith('root:/$ ') for line in lines):
                    break
            assert any(line.startswith('root:/$ ') for line in lines), lines
            hidden_start = len(transcript)
            send('echo SCREEN_ONLY\n')
            for attempt in range(10):
                lines, _ = screenshot('no-serial-mirror')
                if any(line.startswith('SCREEN_ONLY') for line in lines):
                    break
            assert any(line.startswith('SCREEN_ONLY') for line in lines), lines
            send(b'\x1d')
            expect('fbterm: disconnected'); expect('root:/$ ')
            assert b'SCREEN_ONLY' not in transcript[hidden_start:]
            run('ptytest checkflags', 'ptytest: terminal flags restored')
            run('fbterm no-such-fbterm-command', 'fbterm: exec failed')
            # The inner shell and its HUP must not break the original TTY.
            run('stty -a', 'icanon echo')
            assert transcript.count(b'init: starting login') == 1
        print(f'fbterm {args.cpus} CPU: ' + ('GPU absence handled' if args.without_gpu else
              'real pixels, scroll, ls/ps/free, clear/edit, PTY input/signals, EOF/detach and terminal restoration passed'))
    except Exception:
        print(transcript.decode(errors='replace'))
        raise
    finally:
        guest.terminate()
        try:
            guest.wait(timeout=5)
        except subprocess.TimeoutExpired:
            guest.kill()
            guest.wait()
        selector.close()
        (output / f'fbterm-{args.cpus}cpu{"-absent" if args.without_gpu else "-keyboard" if args.keyboard else ""}.log').write_bytes(transcript)
