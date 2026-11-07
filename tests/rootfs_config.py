#!/usr/bin/env python3
"""Install new service defaults without replacing administrator configuration."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from create_rootfs import DIRECTORY, FILE, SYMLINK, Image
from update_rootfs import lookup

parser = argparse.ArgumentParser()
parser.add_argument('--image', required=True, type=Path)
parser.add_argument('--manifest', required=True, type=Path)
args = parser.parse_args()
seed = Image(args.image)
source = lookup(seed, ('etc', 'services.conf'))
default = seed.read(source)


def run(target, *extra, manifest=args.manifest, success=True):
    result = subprocess.run([sys.executable, str(ROOT / 'tools/update_rootfs.py'),
                             '--seed', str(args.image), '--image', str(target),
                             '--manifest', str(manifest), *extra],
                            capture_output=True, text=True)
    assert (result.returncode == 0) == success, result.stdout + result.stderr
    return result.stdout + result.stderr


with tempfile.TemporaryDirectory(prefix='rootfs-config-') as temporary:
    directory = Path(temporary)
    for case in ('missing', 'custom', 'symlink', 'directory'):
        target = directory / (case + '.img')
        image = Image.from_bytes(seed.data)
        etc = lookup(image, ('etc',))
        image.unlink_file(etc, b'services.conf')
        shadow = image.create(etc, b'shadow', FILE, (0, 0, 0o600))
        image.append(shadow, b'PRIVATE_ACCOUNT_DATA\n')
        private = image.create(lookup(image, ('root',)), b'private', FILE, (0, 0, 0o600))
        image.append(private, b'PRIVATE_USER_DATA\n')
        config = None
        if case != 'missing':
            kind = {'custom': FILE, 'symlink': SYMLINK, 'directory': DIRECTORY}[case]
            config = image.create(etc, b'services.conf', kind, (0, 0, 0o600))
            if case == 'custom':
                image.append(config, b'# LOCAL_SERVICE_CONFIGURATION\n')
                image.link(lookup(image, ('root',)), b'config-alias', config)
            elif case == 'symlink':
                image.append(config, b'/root/private')
        target.write_bytes(image.data)
        before = target.read_bytes()
        if case == 'missing':
            assert 'would install: /etc/services.conf' in run(target, '--dry-run')
            assert target.read_bytes() == before
            assert not list(directory.glob(case + '.img.before-update-*.img'))
        output = run(target, success=case != 'directory')
        if case == 'directory':
            assert 'configuration conflicts' in output
            assert target.read_bytes() == before
            assert not list(directory.glob(case + '.img.before-update-*.img'))
            continue
        updated = Image(target)
        installed = lookup(updated, ('etc', 'services.conf'))
        if case == 'missing':
            assert updated.kind(installed) == FILE
            assert updated.read(installed) == default
            assert updated.attributes(installed) == (0, 0, 0o644)
            backups = list(directory.glob(case + '.img.before-update-*.img'))
            assert len(backups) == 1 and backups[0].read_bytes() == before
        else:
            assert installed == config
            assert updated.kind(installed) == image.kind(config)
            assert updated.read(installed) == image.read(config)
            assert updated.attributes(installed) == image.attributes(config)
            assert target.read_bytes() == before
        for inode in (shadow, private):
            assert updated.read(inode) == image.read(inode)
            assert updated.attributes(inode) == image.attributes(inode)
        stable = target.read_bytes()
        assert 'already up to date' in run(target)
        assert target.read_bytes() == stable
    for protected in ('/etc/shadow', '/etc/services.conf.bak', '/etc/services.conf/child'):
        manifest = directory / 'protected.tsv'
        manifest.write_text(args.manifest.read_text() + f'file\t0600\t{protected}\tunused\n')
        assert 'protected path' in run(target, manifest=manifest, success=False)
        assert target.read_bytes() == before

print('Rootfs service defaults: missing installation, custom files/hardlinks/symlinks, '
      'conflicts, protected paths, backups and idempotence PASS')
