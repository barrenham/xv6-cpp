#!/usr/bin/env python3
"""Guest-driven poweroff must reject non-root and leave a durable, clean disk."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from console_login import login_root
from fs_elf_plugins import Guest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qemu', required=True)
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='xv-poweroff-') as temporary:
        image = Path(temporary) / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'poweroff-{args.cpus}cpu.log')
        try:
            login_root(guest.expect, guest.process)
            guest.command('poweroff --help', expect='Usage: poweroff')
            guest.command('poweroff invalid', expect='Usage: poweroff')
            guest.command('useradd poweruser', expect='uid=1000')
            # The app calls the syscall directly; this checks kernel permission.
            guest.command('su poweruser poweroff', expect='poweroff: root required')
            guest.command('echo POWER_OFF_PERSISTED > /root/poweroff-check')
            guest.send('poweroff')
            tail, _ = guest.process.communicate(timeout=25)
            guest.output.extend(tail)
            assert guest.process.returncode == 0, guest.process.returncode
            assert b'poweroff: filesystem sync complete; powering off' in guest.output
            assert b'panic:' not in guest.output
        finally:
            guest.close()
        subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
        sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
        from ext3_image import Ext3Image
        filesystem = Ext3Image.from_bytes(image.read_bytes())
        node = filesystem.root
        for part in (b'root', b'poweroff-check'):
            node = filesystem.entries(node)[part]
        assert filesystem.read(node) == b'POWER_OFF_PERSISTED\n'
    print('poweroff: root permission, guest shutdown, durable data and clean disk PASS')


if __name__ == '__main__':
    main()
