#!/usr/bin/env python3
"""Exercise the real modal editor over UART, then verify its disk contents."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

from console_login import login_root
from fs_elf_plugins import Guest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from ext3_image import Ext3Image


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-vim-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        disk = Ext3Image.from_bytes(image.read_bytes())
        fixtures = {b'vim-original': b'alpha\nbeta\ngamma\n',
                    b'vim-lines': b'top\nbottom',
                    b'vim-scroll': b'A' * 180 + b'TAIL\n' + b''.join(f'line-{i:02d}\n'.encode() for i in range(1, 60)),
                    b'vim-binary': b'a\0b'}
        for name, content in fixtures.items():
            node = disk.create(disk.root, name, 2, (0, 0, 0o640))
            disk.append(node, content)
        disk.close()
        image.write_bytes(disk.data)
        guest = Guest(args, image, args.kernel.parent / f'vim-{args.cpus}cpu.log')
        try:
            login_root(guest.expect, guest.process)
            before = guest.command('stty -a')
            tty_flags = re.sub(r'\x1b\[[0-9;]*m', '', before).splitlines()[0]

            def press(data):
                begin = len(guest.output)
                guest.cursor = begin
                guest.process.stdin.write(data if isinstance(data, bytes) else data.encode())
                guest.process.stdin.flush()
                guest.expect('\x1b[?25h')
                return guest.output[begin:guest.cursor].decode(errors='replace')

            def text(data):
                last = ''
                for value in data:
                    last = press(value)
                return last

            def launch(path=''):
                guest.cursor = len(guest.output)
                command = 'vim' + (' ' + path if path else '')
                guest.send(command)
                guest.expect(command + '\n')
                guest.expect('\x1b[?1049h')
                guest.expect('\x1b[?25h')

            def command(value, quit=False):
                press(':')
                text(value)
                screen = press('\r')
                if quit:
                    guest.expect('\x1b[?1049l')
                    guest.expect('root:/$ ')
                return screen

            def restored():
                after = guest.command('stty -a')
                assert re.sub(r'\x1b\[[0-9;]*m', '', after).splitlines()[0] == tty_flags, after

            launch('/vim-original')
            press('A'); text('!'); press(b'\x1b')
            press('u'); press('u')
            press('/'); text('beta'); screen = press('\r')
            assert '2:1' in screen, repr(screen)
            press('i'); text('new '); press(b'\x1b')
            assert 'No write since last change' in command('q')
            assert 'Written' in command('w')
            assert 'Written' in command('w')
            command('q', quit=True)
            restored()

            launch('/vim-lines')
            press('o'); text('mid'); press(b'\x1b')
            press('O'); text('up'); press(b'\x1b')
            press('x'); press('u'); press('d'); press('d')
            command('wq', quit=True)

            launch('/vim-new')
            press('i'); text('one\r\ttwo'); press(b'\x7f'); text('o')
            press(b'\x1b[H'); press(b'\x1b[3~'); text('  '); press(b'\x1b')
            command('wq', quit=True)
            launch('/vim-new')
            press('A'); text('DISCARD'); press(b'\x1b'); command('q!', quit=True)
            restored()

            launch()
            press('i'); text('unnamed'); press(b'\x1b')
            assert 'Filename missing' in command('w')
            assert 'Written' in command('w /vim-unnamed')
            command('q', quit=True)

            launch('/vim-original')
            press('A'); text('NOT-PUBLISHED'); press(b'\x1b')
            assert 'Destination changed' in command('w /vim-new')
            command('q!', quit=True)

            launch('/proc/vim-denied')
            press('i'); text('rescued'); press(b'\x1b')
            assert 'Cannot create temporary file' in command('w')
            assert 'No write since last change' in command('q')
            assert 'Written' in command('w /vim-rescued')
            command('q', quit=True)

            launch('/vim-scroll')
            assert 'TAIL' in press('$')
            press('0'); press('G'); press(b'\x1b[5~')
            assert '60:1' in press(b'\x1b[6~')
            press('g'); press('g')
            press('d'); press('d'); press('u')
            press('G'); press('A'); text('!'); press(b'\x1b')
            command('wq', quit=True)
            restored()

            # Failed opens leave terminal settings untouched and do not launch UI.
            guest.command('vim /vim-binary', 'Binary files', failure=None)
            guest.command('ln -s /vim-new /vim-link')
            guest.command('vim /vim-link', 'Cannot open file', failure=None)
            guest.command('echo input | vim /vim-original', 'requires a foreground', failure=None)
            guest.command('vim /vim-original > /vim-output', 'requires a foreground', failure=None)
            guest.command('vim --help', 'Vim-style text editor')
            restored()
            guest.command('sync')
        finally:
            guest.close()
        subprocess.run(['e2fsck', '-fn', str(image)], capture_output=True, check=True)
        disk = Ext3Image.from_bytes(image.read_bytes())
        entries = disk.entries(disk.root)
        expected = {b'vim-original': b'alpha!\nnew beta\ngamma\n',
                    b'vim-lines': b'top\nmid\nbottom',
                    b'vim-new': b'one\n  two', b'vim-unnamed': b'unnamed',
                    b'vim-rescued': b'rescued',
                    b'vim-scroll': fixtures[b'vim-scroll'][:-1] + b'!\n',
                    b'vim-binary': fixtures[b'vim-binary']}
        for name, content in expected.items():
            assert disk.read(entries[name]) == content, (name, disk.read(entries[name]), content)
        assert disk.attributes(entries[b'vim-original']) == (0, 0, 0o640)
        assert not any(b'.vim-' in name for name in entries), entries
        disk.close()
    print('vim: modal editing, undo, search, scroll, atomic save, failed saves and termios restore PASS')


if __name__ == '__main__':
    main()
