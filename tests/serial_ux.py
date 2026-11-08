#!/usr/bin/env python3
"""Compare UART edit traffic and verify input, wrapping and job-notice recovery."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

from ansi_screen import Screen
from console_login import login_root
from fs_elf_plugins import Guest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--before-sh', type=Path)
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    records = []
    for label in (('before', 'after') if args.before_sh else ('after',)):
        with tempfile.TemporaryDirectory(prefix='xv-serial-ux-') as temporary:
            image = Path(temporary) / 'root.img'
            shutil.copyfile(args.image, image)
            if label == 'before':
                sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
                from create_rootfs import Image
                disk = Image(image)
                binary = disk.entries(disk.root)[b'bin']
                disk.unlink_file(binary, b'sh')
                node = disk.create(binary, b'sh', 2, (0, 0, 0o755))
                disk.append(node, args.before_sh.read_bytes())
                if hasattr(disk, 'close'): disk.close()
                image.write_bytes(disk.data)
            guest = Guest(args, image, args.kernel.parent / f'serial-ux-{label}-{args.cpus}cpu.log')
            def send(data):
                guest.cursor = len(guest.output)
                data = data if isinstance(data, bytes) else data.encode()
                # Stay below the UART input ring capacity, as a real serial
                # sender does through baud pacing. Do not test FIFO overflow.
                for offset in range(0, len(data), 48):
                    guest.process.stdin.write(data[offset:offset + 48])
                    guest.process.stdin.flush()
                    if offset + 48 < len(data): time.sleep(0.01)
            try:
                login_root(guest.expect, guest.process)
                text = 'a' * 300
                pasted = 'echo ' + text
                for offset in range(0, len(pasted), 48):
                    chunk = pasted[offset:offset + 48]
                    send(chunk)
                    guest.expect(chunk, timeout=60)
                begin = len(guest.output)
                started = time.monotonic()
                for column in range(72, 22, -1):
                    send(b'\x1b[D')
                    guest.expect(f'\r\x1b[{column}C', timeout=20)
                stream = bytes(guest.output[begin:guest.cursor])
                record = {'label': label, 'cpus': args.cpus, 'cursor_moves': 50,
                          'output_bytes': len(stream), 'clear_to_end': stream.count(b'\x1b[J'),
                          'elapsed_seconds': round(time.monotonic() - started, 4)}
                records.append(record)
                if label == 'after':
                    assert not record['clear_to_end'] and record['output_bytes'] <= 800, record
                send(b'X\x05\n')
                guest.expect('\n' + 'a' * 250 + 'X' + 'a' * 50 + '\n')
                guest.expect('root:/$ ')
                if label == 'after':
                    # Exact wrap boundary, insertion and return to the line end.
                    send('echo ' + 'b' * 67)
                    guest.expect('echo ' + 'b' * 67)
                    send(b'\x1b[DX\x05\n')
                    guest.expect('\n' + 'b' * 66 + 'Xb\n')
                    guest.expect('root:/$ ')
                    guest.command('/usr/tests/schedswitch > /tmp/ux-job.log &')
                    send('echo NOTICE_')
                    guest.expect('echo NOTICE_')
                    guest.expect('Done /usr/tests/schedswitch', timeout=90)
                    guest.expect('\x1b[?25h')
                    screen = Screen()
                    screen.feed(guest.output[:guest.cursor])
                    assert screen.text.count('root:/$ echo NOTICE_') == 1, screen.text
                    send('OK\n')
                    guest.expect('\nNOTICE_OK\n'); guest.expect('root:/$ ')
                    guest.command('cat /tmp/ux-job.log', expect='schedswitchtest: PASS')
                    # Input arrives after Enter's acknowledgement but before cat
                    # necessarily starts. Shell batching must leave it in the TTY.
                    send('cat\n'); guest.expect('cat\n')
                    send(b'CHILD_INPUT\n\x04')
                    guest.expect('CHILD_INPUT\n'); guest.expect('root:/$ ', timeout=40)
                    guest.command('echo HANDOFF_OK', expect='HANDOFF_OK')
                guest.send('poweroff')
                tail, _ = guest.process.communicate(timeout=25)
                guest.output.extend(tail)
                assert guest.process.returncode == 0 and b'panic:' not in guest.output
            finally:
                guest.close()
            subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
    if args.before_sh:
        assert records[1]['output_bytes'] < records[0]['output_bytes'] // 4, records
    (args.kernel.parent / f'serial-ux-{args.cpus}cpu.json').write_text(json.dumps(records, indent=2) + '\n')
    print(json.dumps(records), flush=True)
    print('Serial UX: incremental edits, wrap, job notification and foreground input PASS')


if __name__ == '__main__':
    main()
