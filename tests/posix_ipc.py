#!/usr/bin/env python3
"""Exercise POSIX IPC and FS bulk transfers using a disposable root disk."""
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
    with tempfile.TemporaryDirectory(prefix='xv-posix-ipc-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'posix-ipc-{args.cpus}cpu.log')
        try:
            login_root(guest.expect, guest.process)
            print(guest.command('/usr/tests/posixipc', 'posixipc: PASS', timeout=90))
            print(guest.command('/usr/tests/mqtest', 'mqtest: PASS', timeout=90))
            for path in ('/tmp/ipcbench', '/root/ipcbench'):
                output = guest.command('/usr/tests/ipcbench ' + path, 'ipcbench: PASS', timeout=120)
                print(path, output)
                rows = re.findall(r'FSBENCH batch=(\d+) bytes=(\d+) ticks=(\d+) rpc=(\d+) copied=(\d+) waits=(\d+) payload=(\d+)', output)
                if len(rows) != 3:
                    raise AssertionError('missing benchmark rows')
                for batch, size, ticks, rpc, copied, waits, payload in rows:
                    if int(payload) != 16384 or int(rpc) != int(size) // int(batch):
                        raise AssertionError('bulk transport did not coalesce requests: ' + str(rows))
        finally:
            guest.close()
    print('POSIX IPC and shared FS transport: PASS')


if __name__ == '__main__':
    main()
