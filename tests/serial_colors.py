#!/usr/bin/env python3
"""Exercise SGR output, stream detection and colored prompt editing on UART."""
import argparse
from pathlib import Path
import re
import shutil
import tempfile

from console_login import login_root
from fs_elf_plugins import Guest


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--bootstrap', type=Path, required=True)
    parser.add_argument('--image', type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-serial-colors-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'serial-colors-{args.cpus}cpu.log')
        try:
            login_root(guest.expect, guest.process)
            guest.expect('\x1b[0m')
            assert b'\x1b[33mroot:/$ \x1b[0m' in guest.output

            def command(text, colored_prompt=True):
                body = guest.command(text)
                prompt = ('\x1b[33m' if colored_prompt else '') + 'root:/$ '
                assert body.endswith(prompt), repr(body)
                if colored_prompt:
                    guest.expect('\x1b[0m')
                return body[:-len(prompt)]

            command('mkdir /tmp/color-demo')
            command('mkdir /tmp/color-demo/folder')
            command('echo ordinary > /tmp/color-demo/plain')
            command('echo executable > /tmp/color-demo/run')
            command('chmod 755 /tmp/color-demo/run')
            command('ln -s plain /tmp/color-demo/link')
            colored = command('ls /tmp/color-demo')
            assert '\x1b[1mTYPE' in colored
            for code, name in [('34', 'folder'), ('32', 'run'), ('35', 'link')]:
                assert (f'\x1b[35m{name}\x1b[0m' if name == 'link' else f'\x1b[1;{code}m{name}\x1b[0m') in colored, repr(colored)
            assert re.search(r'\splain\r?\n', colored), repr(colored)
            assert '\x1b[1;33mnull\x1b[0m' in command('ls /dev/null')
            assert '\x1b[1mMODE' in command('ls -l --color=auto /tmp/color-demo')
            plain = command('ls --color=never /tmp/color-demo')
            assert re.sub(r'\x1b\[[0-9;]*m', '', colored) == plain

            # Destination controls auto mode, independently of stdin.
            assert '\x1b[' not in command('ls /tmp/color-demo | cat')
            assert '\x1b[1;34mfolder' in command('echo input | ls /tmp/color-demo')
            command('ls /tmp/color-demo > /tmp/color-list')
            assert '\x1b[' not in command('cat /tmp/color-list')
            assert '\x1b[' not in command('env NO_COLOR=1 ls /tmp/color-demo')
            assert '\x1b[' not in command('env TERM=dumb ls /tmp/color-demo')
            command('env NO_COLOR=1 ls --color=always /tmp/color-demo > /tmp/color-list')
            assert '\x1b[1;34mfolder' in command('cat /tmp/color-list')

            command('export NO_COLOR=1', colored_prompt=False)
            assert '\x1b[' not in command('ls /tmp/color-demo', colored_prompt=False)
            command('unset NO_COLOR')
            command('export TERM=dumb', colored_prompt=False)
            command('echo plain-prompt', colored_prompt=False)
            command('unset TERM')

            # Shared presentation covers system, FS, network and account tools.
            for text, marker in [
                ('ps', 'PID'), ('ps -T', 'TID'), ('free', 'total'),
                ('drvctl list', 'MODULE'), ('drvctl devices', 'DEVICE'),
                ('fsctl /', 'Path'), ('mount', 'FILESYSTEM'),
                ('servicectl list', 'NAME'), ('iptables -L', 'Chain'),
            ]:
                body = command(text)
                assert '\x1b[' in body and marker in body, (text, repr(body))
                assert '\x1b[' not in command('env NO_COLOR=1 ' + text)
            for text in ['drvctl --raw list', 'drvctl --raw devices',
                         'fsctl --raw /', 'mount --raw', 'echo raw-data',
                         'pwd', 'env', 'wc /tmp/color-demo/plain',
                         'cat /tmp/color-demo/plain', 'readlink /tmp/color-demo/link']:
                assert '\x1b[' not in command(text), text
            for text in ['rm /tmp/no-such-color-file', 'cp', 'curl', 'useradd']:
                guest.cursor = len(guest.output)
                guest.send(text)
                guest.expect(text + '\n')
                begin = guest.cursor
                guest.expect('root:/$ ')
                body = guest.output[begin:guest.cursor].decode(errors='replace')
                assert '\x1b[1;31m' in body or '\x1b[33m' in body, (text, repr(body))
                guest.expect('\x1b[0m')
            assert '\x1b[32m' in command('useradd coloruser')
            command('userdel coloruser')
            command('free > /tmp/color-list')
            assert '\x1b[' not in command('cat /tmp/color-list')
            assert '\x1b[' not in command('drvctl devices | cat')

            # Cross the serial terminal's 80-column boundary, move left and
            # insert. Match the editor's cursor movement as well as the result.
            text = 'a' * 100
            guest.cursor = len(guest.output)
            for chunk in ['echo ', text[:40], text[40:80], text[80:] + 'END']:
                guest.process.stdin.write(chunk.encode())
                guest.process.stdin.flush()
                guest.expect(chunk)
            guest.process.stdin.write(b'\x1b[D\x1b[D\x1b[D')
            guest.process.stdin.flush()
            guest.expect('\r\x1b[33C')  # 8 prompt + 5 echo + 100 text = column 33.
            guest.process.stdin.write(b'X\x05\n')
            guest.process.stdin.flush()
            guest.expect('\n' + text + 'XEND\n')
            guest.expect('root:/$ ')
            guest.expect('\x1b[0m')
            command('sync')
        finally:
            guest.close()
    print('serial colors: all tests succeeded')


if __name__ == '__main__':
    main()
