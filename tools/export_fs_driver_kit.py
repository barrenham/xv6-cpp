#!/usr/bin/env python3
"""Export user driver SDK/runtime and ABI headers; no kernel implementation."""
import argparse
from pathlib import Path
import shutil


def export(source, target, backend='ext4'):
    source, target = Path(source), Path(target)
    if target.exists():
        raise ValueError('kit destination must not exist')
    target.mkdir(parents=True)
    for directory in ('kernel', 'user/fs/backends'):
        for header in (source / directory).rglob('*'):
            if header.is_file() and header.suffix in ('.h', '.hpp', '.inc'):
                destination = target / header.relative_to(source)
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(header, destination)
    for directory in ('user/runtime', 'user/include', 'user/fs/sdk', f'user/fs/backends/{backend}',
                      f'user/fs/plugins/{backend}', 'linker', 'examples/fs-driver'):
        shutil.copytree(source / directory, target / directory, dirs_exist_ok=True)
    return target


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    export(Path(__file__).resolve().parents[1], args.destination)


if __name__ == '__main__':
    main()
