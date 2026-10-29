#!/usr/bin/env python3
"""Boot private diagnostic kernels for critical-section stack fault boundaries."""
import argparse
import json
import os
from pathlib import Path
import selectors
import shutil
import shlex
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument('--qemu', required=True)
parser.add_argument('--kernel', required=True, type=Path)
parser.add_argument('--image', required=True, type=Path)
a = parser.parse_args()
build = a.kernel.resolve().parent
commands = json.loads((build / 'compile_commands.json').read_text())
compile = next(item for item in commands if item['file'].endswith('/trap/arch/riscv/kernel_platform.cpp'))
link = subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', 'kernel.elf'], text=True).splitlines()[-1]
link = shlex.split(link.split(' && ')[1])
expectations = {1: b'kernel stack locked growth: OK', 2: b'kernel stack: protection fault',
                3: b'kernel stack: emergency frames exhausted', 4: b'kernel stack: guard overflow',
                5: b'kernel virtual pool: 1000 stacks, fragmentation and reuse OK',
                6: b'kernel virtual pool: OOM rollback and recovery OK'}
with tempfile.TemporaryDirectory(prefix='vm-stack-', dir=build) as temporary:
    root = Path(temporary)
    image = root / 'fs.img'
    shutil.copyfile(a.image, image)
    for mode, expected in expectations.items():
        command = shlex.split(compile['command'])
        original = command[command.index('-o') + 1]
        obj = root / f'probe-{mode}.o'
        kernel = root / f'probe-{mode}.elf'
        assert '-DXV_VM_SELF_TEST=0' in command, 'diagnostic compile definition missing'
        command[command.index('-DXV_VM_SELF_TEST=0')] = f'-DXV_VM_SELF_TEST={mode}'
        command[command.index('-o') + 1] = str(obj)
        subprocess.run(command, cwd=compile['directory'], check=True)
        command = [str(obj) if item == original else item for item in link]
        command[command.index('-o') + 1] = str(kernel)
        subprocess.run(command, cwd=compile['directory'], check=True)
        # 1000 stacks also fit when each reservation uses the supported 64 MiB
        # capacity (32 leaf-table pages plus 8 initial data pages per stack).
        ram = '256M' if mode == 5 else '128M'
        guest = subprocess.Popen([a.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(kernel),
            '-m', ram, '-smp', '1', '-nographic', '-snapshot', '-global', 'virtio-mmio.force-legacy=false',
            '-drive', f'file={image},if=none,format=raw,id=x0',
            '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
        transcript = bytearray()
        selector = selectors.DefaultSelector()
        selector.register(guest.stdout, selectors.EVENT_READ)
        try:
            deadline = time.monotonic() + 20
            while expected not in transcript:
                assert guest.poll() is None and time.monotonic() < deadline, transcript.decode(errors='replace')
                for key, _ in selector.select(.1):
                    transcript.extend(os.read(key.fd, 65536))
                if mode in (1, 5, 6):
                    assert b'panic:' not in transcript, transcript.decode(errors='replace')
            print(f'VM kernel probe {mode}: passed', flush=True)
        finally:
            guest.stdin.write(b'\x01x')
            guest.stdin.flush()
            try: guest.wait(timeout=5)
            except subprocess.TimeoutExpired: guest.kill(); guest.wait()
            selector.close()
            (build / f'vm-stack-{mode}.log').write_bytes(transcript)
print('Stack faults, 1000-stack pool reuse and allocation OOM rollback contracts passed')
