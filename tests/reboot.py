#!/usr/bin/env python3
"""Restart the same QEMU twice, retain disk data, then shut down cleanly."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

from console_login import PASSWORD, login_root
from fs_elf_plugins import Guest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-reboot-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'reboot-{args.cpus}cpu.log',
                      network=args.cpus > 1)
        try:
            login_root(guest.expect, guest.process)
            pid = guest.process.pid
            guest.command('reboot --help', expect='Usage: reboot')
            guest.command('reboot invalid', expect='Usage: reboot')
            guest.command('useradd rebootuser', expect='uid=1000')
            guest.command('su rebootuser reboot', expect='reboot: root required')
            for cycle in range(2):
                marker = f'REBOOT_PERSISTED_{cycle}'
                guest.command(f'echo {marker} > /root/reboot-check')
                guest.cursor = len(guest.output)
                guest.send('reboot')
                guest.expect('reboot: filesystem sync complete; rebooting')
                guest.expect('login: ', timeout=60)
                assert guest.process.poll() is None and guest.process.pid == pid
                guest.send('root')
                guest.expect('Password: ')
                guest.send(PASSWORD)
                guest.expect('root:/root$ ', timeout=60)
                guest.command('cat /root/reboot-check', expect=marker)
                guest.command('su rebootuser id', expect='uid=1000(rebootuser)')
                if args.cpus > 1:
                    guest.command('netctl dhcp wait eth0 10', expect='DHCP BOUND')
                    guest.command('ping -q -c 1 -W 1 10.0.2.2',
                                  expect='1 packets transmitted, 1 received')
            guest.send('poweroff')
            tail, _ = guest.process.communicate(timeout=25)
            guest.output.extend(tail)
            assert guest.process.returncode == 0, guest.process.returncode
            assert b'poweroff: filesystem sync complete; powering off' in guest.output
            assert b'panic:' not in guest.output
        finally:
            guest.close()
        subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
    print('reboot: permission, two in-process machine resets, login, durable data and clean poweroff PASS')


if __name__ == '__main__':
    main()
