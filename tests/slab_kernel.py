#!/usr/bin/env python3
"""Measure real slab implementations against page allocation in a private RISC-V kernel."""
import argparse
import json
import os
from pathlib import Path
import re
import selectors
import shlex
import shutil
import subprocess
import tempfile
import time

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--qemu', required=True)
p.add_argument('--kernel', required=True, type=Path)
p.add_argument('--image', required=True, type=Path)
a = p.parse_args()
build = a.kernel.resolve().parent
commands = json.loads((build / 'compile_commands.json').read_text())
source = next(c for c in commands if c['file'].endswith('/trap/arch/riscv/kernel_platform.cpp'))
link = shlex.split(subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', 'kernel.elf'], text=True).splitlines()[-1].split(' && ')[1])
with tempfile.TemporaryDirectory(prefix='slab-kernel-', dir=build) as tmp:
    root = Path(tmp)
    compile = shlex.split(source['command'])
    original = compile[compile.index('-o') + 1]
    obj, kernel, image = root/'probe.o', root/'kernel.elf', root/'fs.img'
    compile[compile.index('-o') + 1] = str(obj)
    compile[compile.index('-DXV_VM_SELF_TEST=0')] = '-DXV_VM_SELF_TEST=7'
    subprocess.run(compile, cwd=source['directory'], check=True)
    link = [str(obj) if part == original else part for part in link]
    link[link.index('-o') + 1] = str(kernel)
    subprocess.run(link, cwd=source['directory'], check=True)
    shutil.copyfile(a.image, image)
    guest = subprocess.Popen([a.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(kernel),
        '-m', '128M', '-smp', '1', '-nographic', '-snapshot', '-global', 'virtio-mmio.force-legacy=false',
        '-drive', f'file={image},if=none,format=raw,id=x0',
        '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0'],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=0)
    data = bytearray()
    selector = selectors.DefaultSelector()
    selector.register(guest.stdout, selectors.EVENT_READ)
    try:
        deadline = time.monotonic() + 60
        while b'slab kernel performance: OK' not in data:
            if guest.poll() is not None or time.monotonic() > deadline or b'panic:' in data:
                raise RuntimeError(data.decode(errors='replace'))
            for key, _ in selector.select(.1):
                data.extend(os.read(key.fd, 65536))
        rows = []
        for name, direct, slab, calls in re.findall(rb'slab kernel perf: name=(\S+) page_ticks=(\d+) slab_ticks=(\d+) page_calls=(\d+)', data):
            direct, slab, calls = int(direct), int(slab), int(calls)
            assert slab * 10 < direct * 9 and calls == 1
            rows.append(dict(implementation=name.decode(), page_ticks=direct, slab_ticks=slab,
                             ratio=slab/direct, page_calls=calls, rounds=5, operations_per_round=1024))
        assert len(rows) == 2 and {r['implementation'] for r in rows} == {'freelist-slab', 'bitmap-slab'}
        (build/'slab-kernel-performance.json').write_text(json.dumps(rows, indent=2)+'\n')
        print(json.dumps(rows), flush=True)
    finally:
        guest.terminate()
        try: guest.wait(timeout=5)
        except subprocess.TimeoutExpired: guest.kill(); guest.wait()
        selector.close()
        (build/'slab-kernel-performance.log').write_bytes(data)
