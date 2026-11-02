#!/usr/bin/env python3
"""Produce a deterministic on-disk USTAR sample for the generic FS bridge."""
import argparse
import io
from pathlib import Path
import tarfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(args.output, 'w', format=tarfile.USTAR_FORMAT) as archive:
        for name, content in [
            ('hello.txt', b'Hello from an independently registered archive filesystem.\n'),
            ('docs/readme.txt', b'The kernel forwards operations; tarfsd reads this archive through an open descriptor.\n'),
        ]:
            info = tarfile.TarInfo(name)
            info.size = len(content)
            info.mode = 0o444
            info.mtime = 0
            archive.addfile(info, io.BytesIO(content))


if __name__ == '__main__':
    main()
