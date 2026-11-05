#!/usr/bin/env python3
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--qemu', required=True)
parser.add_argument('--contract', required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-dt-contract-') as temporary:
    blob = Path(temporary) / 'machine.dtb'
    subprocess.run([args.qemu, '-machine', f'virt,dumpdtb={blob}', '-bios', 'none',
                    '-m', '128M', '-smp', '5', '-nographic'], check=True, timeout=15)
    subprocess.run([args.contract, str(blob)], check=True, timeout=15)
