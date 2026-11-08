#!/usr/bin/env python3
"""Exercise lazy file pages/COW, unified cache, FS workers and service supervision."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
from console_login import login_root
from fs_elf_plugins import Guest

parser = argparse.ArgumentParser()
for option in ('kernel', 'bootstrap', 'image', 'mkfs'):
    parser.add_argument('--' + option, type=Path, required=True)
parser.add_argument('--qemu', required=True)
parser.add_argument('--cpus', type=int, required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-system-extensions-') as temporary:
    directory = Path(temporary)
    image = directory / 'root.img'
    shutil.copyfile(args.image, image)
    data = directory / 'legacy.img'
    manifest = directory / 'manifest.tsv'
    manifest.write_text('dir\t0755\t/volume\n')
    subprocess.run([str(args.mkfs), str(data), '--manifest', str(manifest),
                    '--filesystem', 'xv6'], check=True)
    guest = Guest(args, image, args.kernel.parent / f'system-extensions-{args.cpus}cpu.log', data=data)
    try:
        login_root(guest.expect, guest.process)
        def table(command, header):
            # Boot-supervisor diagnostics may split a command's echoed input
            # during crash recovery. Synchronize on its actual table instead.
            guest.cursor = len(guest.output)
            guest.send('env NO_COLOR=1 ' + command)
            guest.expect(header)
            begin = guest.cursor
            guest.expect('$ ')
            return guest.output[begin:guest.cursor].decode(errors='replace')

        # This test parses table fields; color presentation has its own UART
        # coverage. Request the plain interface rather than counting SGR bytes.
        listing = guest.command('env NO_COLOR=1 ps -T', 'CPU_MS')
        tasks = [line.split() for line in listing.splitlines()
                 if re.match(r'^\s*\d+\s+\d+\s+[A-Z?]\s+', line)]
        for name in ('ext3.elf', 'devfs.elf', 'procfs.elf', 'ramfs.elf'):
            own = [task for task in tasks if task[-1] == name]
            assert len(own) == 3 and len({task[0] for task in own}) == 1, (name, own)
            assert len({task[1] for task in own}) == 3, own
        guest.command('mkdir /tmp/worker-options')
        guest.command('mount -t ramfs -o workers=0 /tmp/worker-options', failure=True)
        guest.command('mount -t ramfs -o read-workers=2 /tmp/worker-options', failure=True)
        mounted = guest.command('mount -t ramfs -o workers=1 /tmp/worker-options')
        provider = re.search(r'server pid=(\d+)', mounted).group(1)
        listing = guest.command('env NO_COLOR=1 ps -T ' + provider, 'CPU_MS')
        assert len([line for line in listing.splitlines()
                    if re.match(r'^\s*' + provider + r'\s+\d+\s+', line)]) == 2, listing
        guest.command('umount /tmp/worker-options')
        guest.command('rm /tmp/worker-options')
        print(guest.command('/usr/tests/fsworkertest', 'fsworkertest: PASS', timeout=60))
        for path in ('/tmp/demandcache', '/root/demandcache'):
            print(guest.command('/usr/tests/demandcachetest ' + path, 'demandcachetest: PASS', timeout=120))
        print(guest.command('/usr/tests/servicetest', 'servicetest: PASS', timeout=120))
        output = guest.command('servicectl list', 'rootfs')
        assert 'NAME' in output and 'STATE' in output and 'driver-autoload' in output, output
        guest.command('echo classified-apps | grep classified-apps', 'classified-apps')
        guest.command('drvctl devices', 'DEVICE_ID')
        guest.command('mkdir /tmp/parallel-ramfs')
        guest.command('mount -t ramfs -o workers=2 /tmp/parallel-ramfs')
        print(guest.command('/usr/tests/demandcachetest /tmp/parallel-ramfs/data',
                            'demandcachetest: PASS', timeout=120))
        guest.command('umount /tmp/parallel-ramfs')
        guest.command('mkdir /tmp/legacy')
        guest.command('mount -t xv6 -s 1 /tmp/legacy')
        print(guest.command('/usr/tests/demandcachetest /tmp/legacy/data',
                            'demandcachetest: PASS', timeout=120))
        guest.command('umount /tmp/legacy')
        managers = [line.split()[0] for line in table('ps', 'PID').splitlines()
                    if line.split() and line.split()[-1] == 'serviced']
        assert len(managers) == 1, managers
        guest.command('kill -KILL ' + managers[0])
        # UART diagnostics can interleave the shell prompt in the middle of a
        # message. Verify actual process/control recovery instead of its text.
        deadline = time.monotonic() + 15
        while True:
            after = [line.split()[0] for line in table('ps', 'PID').splitlines()
                     if line.split() and line.split()[-1] == 'serviced']
            if len(after) == 1 and after[0] != managers[0]:
                state = table('servicectl list', 'NAME')
                if 'rootfs' in state and 'driver-autoload' in state:
                    break
            assert time.monotonic() < deadline, (after, guest.output.decode(errors='replace'))
            time.sleep(.1)
        guest.command('ls -l /tmp/serviced.log.1', '65536')
        guest.command('echo login-survives', 'login-survives')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
print('System extensions: lazy files/COW, unified bounded cache, actual FS workers and service supervisor PASS')
