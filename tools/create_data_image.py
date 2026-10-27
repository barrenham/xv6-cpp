#!/usr/bin/env python3
import argparse, os, subprocess, tempfile
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument('--mkfs',required=True); p.add_argument('--output',required=True); a=p.parse_args()
output=Path(a.output)
if not output.exists():
    output.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='xv-data-',dir=output.parent) as temporary:
        manifest=Path(temporary)/'manifest.tsv'
        manifest.write_text('dir\t0755\t/etc\ndir\t0755\t/home\ndir\t0700\t/root\n')
        image=Path(temporary)/'data.img'
        subprocess.run([a.mkfs,str(image),'--manifest',str(manifest)],check=True)
        try: os.link(image,output)  # Never replace an existing user's volume.
        except FileExistsError: pass
