#!/usr/bin/env python3
"""Exercise lazy file pages/COW, unified cache, FS workers and service supervision."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
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
        print(guest.command('/usr/tests/fsworkertest', 'fsworkertest: PASS', timeout=60))
        for path in ('/tmp/demandcache', '/root/demandcache'):
            print(guest.command('/usr/tests/demandcachetest ' + path, 'demandcachetest: PASS', timeout=120))
        print(guest.command('/usr/tests/servicetest', 'servicetest: PASS', timeout=120))
        output = guest.command('servicectl list', 'rootfs')
        assert 'NAME' in output and 'STATE' in output and 'driver-autoload' in output, output
        guest.command('echo classified-apps | grep classified-apps', 'classified-apps')
        guest.command('drvctl devices', 'DEVICE_ID')
        guest.command('mkdir /tmp/parallel-ramfs')
        guest.command('mount -t ramfs -o read-workers=2 /tmp/parallel-ramfs')
        print(guest.command('/usr/tests/demandcachetest /tmp/parallel-ramfs/data',
                            'demandcachetest: PASS', timeout=120))
        guest.command('umount /tmp/parallel-ramfs')
        guest.command('mkdir /tmp/legacy')
        guest.command('mount -t xv6 -s 1 /tmp/legacy')
        print(guest.command('/usr/tests/demandcachetest /tmp/legacy/data',
                            'demandcachetest: PASS', timeout=120))
        guest.command('umount /tmp/legacy')
        managers = [line.split()[0] for line in guest.command('ps').splitlines()
                    if line.split() and line.split()[-1] == 'serviced']
        assert len(managers) == 1, managers
        guest.command('kill -KILL ' + managers[0])
        guest.cursor = 0
        guest.expect('fsboot: service manager service pid=' + managers[0] + ' exited')
        assert not any(line.split() and line.split()[-1] == 'serviced'
                       for line in guest.command('ps').splitlines())
        guest.command('servicectl list', 'servicectl: manager unavailable')
        guest.command('echo login-survives', 'login-survives')
        guest.command('sync')
    finally:
        guest.close()
    subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
print('System extensions: lazy files/COW, unified bounded cache, actual FS workers and service supervisor PASS')
