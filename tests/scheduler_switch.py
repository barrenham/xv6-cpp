#!/usr/bin/env python3
"""Live global policy changes under CPU, pthread, sleeping and filesystem load."""
import argparse
from pathlib import Path
import re
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
    parser.add_argument('--initial-policy', choices=('cfs', 'fair', 'mlfq', 'round-robin'), default='cfs')
    args = parser.parse_args()
    initial_policy = 'cfs' if args.initial_policy == 'fair' else args.initial_policy
    with tempfile.TemporaryDirectory(prefix='xv-scheduler-switch-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'scheduler-switch-{args.cpus}cpu.log')
        try:
            login_root(guest.expect, guest.process)
            initial = guest.command('schedctl', expect='Policy:    ' + initial_policy)
            assert re.search(r'Changes:\s+0', initial), initial
            guest.command('schedctl --help', expect='Usage: schedctl')
            guest.command('schedctl invalid', expect='Usage: schedctl')
            guest.command('useradd scheduser', expect='uid=1000')
            guest.command('su scheduser schedctl show', expect='Policy:    ' + initial_policy)
            guest.command('su scheduser schedctl mlfq', expect='schedctl: root required')
            for policy in ('mlfq', 'round-robin', 'cfs'):
                guest.command('schedctl ' + policy, expect='Policy:    ' + policy)
                guest.command(f'/usr/tests/schedulercheck {max(3, 2 * args.cpus)}',
                              expect='schedulercheck: all workers made bounded progress', timeout=70)
            guest.command('/usr/tests/schedswitch', expect='schedswitchtest: PASS', timeout=180)
            guest.command('schedctl show', expect='Policy:    cfs')
            guest.command('echo SCHEDULER_DISK_ALIVE > /root/scheduler-switch-check')
            guest.command('cat /root/scheduler-switch-check', expect='SCHEDULER_DISK_ALIVE')
            other = 'round-robin' if initial_policy == 'mlfq' else 'mlfq'
            guest.command('schedctl ' + other, expect='Policy:    ' + other)
            guest.cursor = len(guest.output)
            guest.send('reboot')
            guest.expect('reboot: filesystem sync complete; rebooting')
            guest.expect('login: ', timeout=60)
            guest.send('root')
            guest.expect('Password: ')
            guest.send(PASSWORD)
            guest.expect('root:/root$ ', timeout=60)
            reset = guest.command('schedctl', expect='Policy:    ' + initial_policy)
            assert re.search(r'Changes:\s+0', reset), reset
            guest.command('cat /root/scheduler-switch-check', expect='SCHEDULER_DISK_ALIVE')
            guest.send('poweroff')
            tail, _ = guest.process.communicate(timeout=25)
            guest.output.extend(tail)
            assert guest.process.returncode == 0
            assert b'poweroff: filesystem sync complete; powering off' in guest.output
            assert b'panic:' not in guest.output
        finally:
            guest.close()
        subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
    print('Live scheduler switching: CLI, permissions, all policies, concurrency and clean shutdown PASS')


if __name__ == '__main__':
    main()
