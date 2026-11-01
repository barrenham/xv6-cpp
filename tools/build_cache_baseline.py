#!/usr/bin/env python3
"""Build an isolated, equally instrumented linear-cache comparison kernel."""
import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--clang', default=shutil.which('clang++'))
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    output = args.output.resolve()
    if output == ROOT or ROOT.is_relative_to(output):
        parser.error('output must not contain the development repository')
    if output.is_relative_to(ROOT) and not output.is_relative_to(ROOT / 'build'):
        parser.error('in-repository output must be inside build/')
    source = output / 'source'
    source.mkdir(parents=True, exist_ok=True)
    for directory in ['kernel', 'tests', 'cmake', 'tools/mkfs']:
        shutil.copytree(ROOT / directory, source / directory, dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns('__pycache__', 'CMakeFiles', 'host_mkfs-prefix'))
    for name in ['user', 'linker']:
        path = source / name
        if not path.exists():
            path.symlink_to(ROOT / name, target_is_directory=True)
    for name in ['CMakeLists.txt', 'README.xv6']:
        shutil.copyfile(ROOT / name, source / name)
    patch = (ROOT / 'docs/block-cache-linear-baseline.patch').read_bytes()
    subprocess.run(['patch', '-p1', '--batch'], input=patch, cwd=source, check=True)
    subprocess.run(['cmake', '-S', str(source), '-B', str(output / 'kernel'), '-G', 'Ninja',
                    f'-DCMAKE_TOOLCHAIN_FILE={source}/cmake/riscv-clang.cmake',
                    '-DXV_PAGE_ALLOCATOR=buddy', '-DXV_SCHEDULER=fair'], check=True)
    subprocess.run(['cmake', '--build', str(output / 'kernel'), '--target', 'kernel'], check=True)
    if args.native:
        subprocess.run(['cmake', '-S', str(source / 'tools/mkfs/host'), '-B', str(output / 'host'),
                        '-G', 'Ninja', f'-DCMAKE_CXX_COMPILER={args.clang}',
                        f'-DXV_OUTPUT_DIR={output}/native'], check=True)
        subprocess.run(['cmake', '--build', str(output / 'host'), '--target', 'fs_contract'], check=True)


if __name__ == '__main__':
    main()
