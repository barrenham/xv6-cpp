"""Independent ext4 fixtures produced by e2fsprogs, never the guest writer."""
from pathlib import Path
import re
import shutil
import subprocess


def command(argv, **kwargs):
    return subprocess.run(argv, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)


def make_image(directory, block, *, checksums=True, seed=False, bits64=True, executable=None):
    directory = Path(directory)
    tree = directory / 'tree'
    tree.mkdir()
    (tree / 'message').write_bytes(b'EXT4_READ_ONLY_OKAY\n')
    blob = bytes((i * 37 + 11) % 256 for i in range(256))
    (tree / 'blob').write_bytes((blob * ((2 * 1024 * 1024 + 17) // 256 + 1))[:2 * 1024 * 1024 + 17])
    (tree / 'fragmented').write_bytes((blob * (64 * block // 256 + 1))[:64 * block])
    (tree / 'private').write_bytes(b'PRIVATE\n')
    (tree / 'sub').mkdir()
    (tree / 'sub' / ('n' * 80)).write_bytes(b'LONG_LINK_OKAY\n')
    (tree / 'shortlink').symlink_to('message')
    (tree / 'longlink').symlink_to('sub/' + 'n' * 80)
    large = tree / 'large'
    large.mkdir()
    for i in range(1600):
        (large / (f'{i:04d}' + 'n' * 176)).write_bytes(bytes([i & 255]))
    if executable:
        shutil.copyfile(executable, tree / 'probe')
        (tree / 'probe').chmod(0o755)
    image = directory / 'ext4.img'
    with image.open('wb') as stream:
        stream.truncate(64 * 1024 * 1024)
    features = []
    if not checksums:
        features += ['^metadata_csum', 'uninit_bg']
    if seed:
        features += ['metadata_csum_seed']
    if not bits64:
        features += ['^64bit']
    options = ['-O', ','.join(features)] if features else []
    command([shutil.which('mke2fs') or '/usr/sbin/mke2fs', '-q', '-F', '-t', 'ext4',
             '-b', str(block), '-N', '4096', '-d', str(tree), *options, str(image)])
    commands = directory / 'debugfs.txt'
    lines = [f'punch /fragmented {i} {i}' for i in range(1, 16, 2)]
    empty = directory / 'empty'; empty.write_bytes(b'')
    lines += [f'write {empty} /unwritten', 'fallocate /unwritten 0 31',
              f'set_inode_field /unwritten size {32 * block}',
              'set_inode_field /private uid 0', 'set_inode_field /private gid 0',
              'set_inode_field /private mode 0100600']
    commands.write_text('\n'.join(lines) + '\n')
    command([shutil.which('debugfs') or '/usr/sbin/debugfs', '-w', '-f', str(commands), str(image)])
    # Rebuild htrees and all metadata checksums independently.
    result = subprocess.run([shutil.which('e2fsck') or '/usr/sbin/e2fsck', '-fyD', str(image)], capture_output=True)
    if result.returncode not in (0, 1):
        raise AssertionError(result.stdout + result.stderr)
    leaves = command([shutil.which('debugfs') or '/usr/sbin/debugfs',
                      '-R', 'dump_extents -l /unwritten', str(image)]).stdout.decode()
    numbers = []
    for row in leaves.splitlines():
        match = re.match(r'^\s*\d+/\s*\d+\s+\d+/\s*\d+\s+\d+\s*-\s*\d+\s+(\d+)\s*-\s*(\d+)\s+(\d+)', row)
        if match:
            first, last, count = map(int, match.groups())
            assert last - first + 1 == count and 'Uninit' in row, row
            numbers.extend(range(first, last + 1))
    assert len(numbers) == 32, leaves
    assert numbers
    with image.open('r+b') as stream:
        # Unwritten extents must read as zero despite nonzero physical contents.
        for number in numbers:
            stream.seek(number * block)
            stream.write(b'\x7e' * block)
    command([shutil.which('e2fsck') or '/usr/sbin/e2fsck', '-fn', str(image)])
    return image
