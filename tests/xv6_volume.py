#!/usr/bin/env python3
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--mkfs', required=True)
parser.add_argument('--contract', required=True)
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='xv-user-volume-') as name:
    directory = Path(name)
    manifest = directory / 'manifest'
    manifest.write_text('')
    image = directory / 'disk.img'
    subprocess.run([args.mkfs, str(image), '--manifest', str(manifest), '--filesystem', 'xv6'], check=True)
    subprocess.run([args.contract, str(image)], check=True)
