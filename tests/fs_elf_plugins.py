#!/usr/bin/env python3
"""Build a plugin outside the repository, install it, and exercise discovery."""
import argparse
import hashlib
import os
from pathlib import Path
import selectors
import shutil
import subprocess
import sys
import tempfile
import time

from console_login import PASSWORD, login_root


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class Guest:
    def __init__(self, args, image, log, data=None, network=False, extra=()):
        self.output = bytearray()
        self.cursor = 0
        self.log = log
        command = [
            args.qemu, '-machine', 'virt', '-bios', 'none', '-kernel', str(args.kernel),
            '-initrd', str(args.bootstrap), '-m', '128M', '-smp', str(args.cpus),
            '-nographic', '-global', 'virtio-mmio.force-legacy=false',
            '-drive', f'file={image},if=none,format=raw,id=x0',
            '-device', 'virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0',
            '-object', 'rng-random,id=rng0,filename=/dev/urandom',
            '-device', 'virtio-rng-device,rng=rng0,bus=virtio-mmio-bus.2']
        if data is not None:
            command += ['-drive', f'file={data},if=none,format=raw,id=x1',
                        '-device', 'virtio-blk-device,drive=x1,bus=virtio-mmio-bus.1']
        if network:
            command += ['-netdev', 'user,id=net0,dhcpstart=10.0.2.100', '-device',
                        'e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56']
        command += list(extra)
        self.process = subprocess.Popen(command,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            env=dict(os.environ, TMPDIR=str(image.parent)))
        self.selector = selectors.DefaultSelector()
        self.selector.register(self.process.stdout, selectors.EVENT_READ)

    def expect(self, token, timeout=40):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            for failure in (b'panic:', b'usertrap(): unexpected', b'fsboot: root service '):
                if failure in self.output:
                    raise RuntimeError('guest failed: ' + failure.decode())
            found = self.output.find(token.encode(), self.cursor)
            if found >= 0:
                self.cursor = found + len(token)
                return
            if self.process.poll() is not None:
                raise RuntimeError('guest exited before expected output')
            for key, _ in self.selector.select(.1):
                data = os.read(key.fd, 65536)
                if data:
                    self.output.extend(data)
                    self.log.write_bytes(self.output)
        raise RuntimeError(f'timeout waiting for {token!r}')

    def send(self, command):
        self.process.stdin.write((command + '\n').encode())
        self.process.stdin.flush()

    def command(self, command, expect=None, failure=False, timeout=40):
        self.cursor = len(self.output)
        self.send(command)
        self.expect(command + '\n', timeout=10)
        begin = self.cursor
        self.expect('$ ', timeout=timeout)
        body = self.output[begin:self.cursor].decode(errors='replace')
        if expect is not None and expect not in body:
            raise AssertionError(f'{command}: missing {expect!r}: {body}')
        if failure is True:
            if 'failed' not in body and 'cannot' not in body:
                raise AssertionError(f'{command}: expected failure: {body}')
        elif failure is False and any(marker in body for marker in ('mount: failed', 'cannot open', 'cannot execute')):
            raise AssertionError(f'{command}: unexpected failure: {body}')
        return body

    def close(self):
        if self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        self.selector.close()
        self.process.stdin.close()
        self.process.stdout.close()
        self.log.write_bytes(self.output)


def build_plugin(args, directory):
    sdk = directory / 'sdk'
    sdk.mkdir()
    for component in ('include', 'standalone'):
        shutil.copytree(args.sdk / component, sdk / component)
    example = directory / 'external-source'
    shutil.copytree(args.example, example)
    copied = example / 'hello.c'
    source = copied.read_text().replace('registration.max_payload = XVFS_DATA_SIZE;',
                                      'registration.max_payload = 64;')
    malformed_reply = """    case XVFS_CONTROL:
      if (equal(message.name, HELLO_INTERFACE) && message.other == 65535)
      { message.count = 8; message.result = 8; break; }"""
    copied.write_text(source.replace('    case XVFS_CONTROL:', malformed_reply))
    build = directory / 'external-build'
    subprocess.run([args.cmake, '-S', str(example), '-B', str(build), '-G', 'Ninja',
                    f'-DXVFS_SDK={sdk}',
                    f'-DCMAKE_TOOLCHAIN_FILE={sdk / "standalone/riscv64-toolchain.cmake"}'], check=True)
    subprocess.run([args.cmake, '--build', str(build), '-j', '1'], check=True)
    hang = example / 'hang.c'
    hang.write_text('int main(void) { for (;;) __asm__ volatile("" ::: "memory"); }\n')
    stalled = directory / 'hang-build'
    subprocess.run([args.cmake, '-S', str(example), '-B', str(stalled), '-G', 'Ninja',
                    f'-DXVFS_SDK={sdk}', f'-DXVFS_PLUGIN_SOURCE={hang}',
                    f'-DCMAKE_TOOLCHAIN_FILE={sdk / "standalone/riscv64-toolchain.cmake"}'], check=True)
    subprocess.run([args.cmake, '--build', str(stalled), '-j', '1'], check=True)
    shutil.copyfile(stalled / 'hello.elf', build / 'hang.elf')
    return build


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    parser.add_argument('--kernel', required=True, type=Path)
    parser.add_argument('--bootstrap', required=True, type=Path)
    parser.add_argument('--image', required=True, type=Path)
    parser.add_argument('--mkfs', required=True, type=Path)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--example', required=True, type=Path)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--cpus', required=True, type=int)
    args = parser.parse_args()
    for key in ('kernel', 'bootstrap', 'image', 'mkfs', 'sdk', 'example'):
        setattr(args, key, getattr(args, key).resolve())
    unchanged = {path: digest(path) for path in (args.kernel, args.bootstrap)}
    repo = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(repo / 'tools'))
    from create_rootfs import Image
    from update_rootfs import lookup, update
    with tempfile.TemporaryDirectory(prefix='xv-external-plugin-') as temporary:
        directory = Path(temporary)
        plugin = build_plugin(args, directory)
        image = directory / 'root.img'
        shutil.copyfile(args.image, image)
        guest = Guest(args, image, args.kernel.parent / f'elf-plugin-{args.cpus}cpu-enroll.log')
        try:
            login_root(guest.expect, guest.process)
            guest.command('echo KEEP_PRIVATE > /data/mod-keep')
            guest.command('sync')
        finally:
            guest.close()
        original = Image(image)
        protected = {}
        for name in ('/etc/passwd', '/etc/shadow', '/etc/group', '/data/mod-keep'):
            number = lookup(original, Path(name).parts[1:])
            protected[name] = (original.attributes(number), original.read(number))
        del original
        entries = ['dir\t0755\t' + path for path in
                   ('/usr', '/usr/lib', '/usr/lib/fsplugins', '/usr/share',
                    '/usr/share/doc', '/usr/share/doc/xv6', '/etc', '/etc/fsplugins')]
        entries.append(f'file\t0755\t/usr/lib/fsplugins/mod.elf\t{plugin / "hello.elf"}')
        entries.append(f'file\t0755\t/usr/lib/fsplugins/hang.elf\t{plugin / "hang.elf"}')
        fixtures = {
            'sdkmod.conf': 'abi=1\ntype=sdkmod\nalias=sdkalias\nexec=/usr/lib/fsplugins/mod.elf\nresource=none\nmode=0555\n',
            'fileview.conf': 'abi=1\ntype=fileview\nexec=/usr/lib/fsplugins/mod.elf\nresource=file\nsource=/usr/share/doc/xv6/modsrc\naccess=rw\nmode=0555\n',
            'badabi.conf': 'abi=99\ntype=badabi\nexec=/usr/lib/fsplugins/mod.elf\nresource=none\n',
            'slow.conf': 'abi=1\ntype=slowmod\nexec=/usr/lib/fsplugins/mod.elf\nresource=none\noptions=slowstart\nstartup_ms=45000\n',
            'hang.conf': 'abi=1\ntype=hangmod\nexec=/usr/lib/fsplugins/hang.elf\nresource=none\nstartup_ms=1500\n',
            'badsource.conf': 'abi=1\ntype=badsource\nexec=/usr/lib/fsplugins/bad.elf\nresource=file\n',
            'badelf.conf': 'abi=1\ntype=badelf\nexec=/usr/lib/fsplugins/bad.elf\nresource=none\n',
        }
        for name, content in fixtures.items():
            source = directory / name
            source.write_text(content)
            entries.append(f'file\t0644\t/etc/fsplugins/{name}\t{source}')
        source = directory / 'source.txt'
        source.write_text('FILE_SOURCE\n')
        entries.append(f'file\t0644\t/usr/share/doc/xv6/modsrc\t{source}')
        bad = directory / 'bad.elf'
        bad.write_bytes(b'not an ELF\n')
        entries.append(f'file\t0755\t/usr/lib/fsplugins/bad.elf\t{bad}')
        duplicate = directory / 'duplicate.conf'
        duplicate.write_text(fixtures['sdkmod.conf'])
        entries.append(f'file\t0644\t/usr/share/doc/xv6/duplicate\t{duplicate}')
        manifest = directory / 'install.tsv'
        manifest.write_text('\n'.join(entries) + '\n')
        seed = directory / 'plugins.img'
        subprocess.run([str(args.mkfs), str(seed), '--manifest', str(manifest), '--filesystem', 'ext3'], check=True)
        update(seed, image, manifest)
        updated = Image(image)
        for name, expected in protected.items():
            number = lookup(updated, Path(name).parts[1:])
            assert (updated.attributes(number), updated.read(number)) == expected, name
        del updated
        guest = Guest(args, image, args.kernel.parent / f'elf-plugin-{args.cpus}cpu-runtime.log')
        try:
            guest.expect('login: ')
            guest.send('root')
            guest.expect('Password: ')
            guest.send(PASSWORD)
            guest.expect('root:/root$ ')
            guest.command('cd /')
            processes = guest.command('ps')
            for executable in ('devfs.elf', 'procfs.elf', 'ramfs.elf'):
                assert executable in processes, (executable, processes)
            guest.command('cat /data/mod-keep', 'KEEP_PRIVATE')
            guest.command('mkdir /tmp/mod')
            guest.command('mount -t sdkmod -o SDK_OPTIONS /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'SDK_OPTIONS')
            guest.command('umount /tmp/mod')
            guest.command('mount -t sdkalias -o ALIAS_OPTIONS /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'ALIAS_OPTIONS')
            guest.command('umount /tmp/mod')
            guest.command('mount -t badabi /tmp/mod', failure=True)
            guest.command('mount -t badelf /tmp/mod', failure=True)
            guest.command('mount -t hangmod /tmp/mod', failure=True)
            guest.command('mount -t slowmod /tmp/mod', timeout=65)
            guest.command('cat /tmp/mod/hello', 'slowstart')
            guest.command('umount /tmp/mod')
            guest.command('cp /usr/share/doc/xv6/duplicate /etc/fsplugins/duplicate.conf')
            guest.command('mount -t sdkmod /tmp/mod', failure=True)
            guest.command('rm /etc/fsplugins/duplicate.conf')
            guest.command('mount -t sdkmod -o RECOVERED /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'RECOVERED')
            guest.command('umount /tmp/mod')
            guest.command('mount -t fileview /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'FILE_SOURCE')
            guest.command('umount /tmp/mod')
            guest.command('mkdir /tmp/source')
            guest.command('mount -t ramfs /tmp/source')
            guest.command('echo SOURCE_OVERRIDE > /tmp/source/input')
            guest.command('mount -t badsource -s /tmp/source/input /tmp/mod', failure=True)
            guest.command('mount -t fileview -s /tmp/source/input -o rw /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'SOURCE_OVERRIDE')
            guest.command('echo APPENDED >> /tmp/mod/hello')
            guest.command('cat /tmp/source/input', 'APPENDED')
            guest.command('umount /tmp/source', failure=True)
            guest.command('umount /tmp/mod')
            for attempt in range(50):
                body = guest.command('umount /tmp/source', failure=False if attempt == 49 else None)
                if 'umount: failed' not in body:
                    break
                time.sleep(.1)
            else:
                raise AssertionError('source remained pinned after plugin shutdown')
            for _ in range(20):
                guest.command('mount -t sdkmod /tmp/mod')
                guest.command('cat /tmp/mod/hello', 'independently built ELF')
                guest.command('umount /tmp/mod')
            if b'fsboot: ext3 mounted at /,' in guest.output:
                guest.command('fsdrivertest ext3 /', 'fsdrivertest: PASS: ext3')
            else:
                guest.command('fsdrivertest unsupported /', 'fsdrivertest: PASS: unsupported')
            guest.command('fsdrivertest unsupported /tmp', 'fsdrivertest: PASS: unsupported')
            guest.command('mount -t sdkmod /tmp/mod')
            guest.command('fsctl /tmp/mod/hello', 'Interface       : example.file')
            guest.command('fsdrivertest readonly /tmp/mod/hello', 'fsdrivertest: PASS: readonly')
            guest.command('umount /tmp/mod')
            guest.command('mount -t fileview -o rw /tmp/mod')
            guest.command('fsdrivertest external /tmp/mod/hello', 'fsdrivertest: PASS: external')
            guest.command('cat /usr/share/doc/xv6/modsrc', 'NATIVE_WRITE_PERSISTED')
            guest.command('umount /tmp/mod')
            guest.command('sync')
        finally:
            guest.close()
        guest = Guest(args, image, args.kernel.parent / f'elf-plugin-{args.cpus}cpu-native-reboot.log')
        try:
            guest.expect('login: ')
            guest.send('root')
            guest.expect('Password: ')
            guest.send(PASSWORD)
            guest.expect('root:/root$ ')
            guest.command('cd /')
            guest.command('mkdir /tmp/mod')
            guest.command('mount -t fileview /tmp/mod')
            guest.command('cat /tmp/mod/hello', 'NATIVE_WRITE_PERSISTED')
            guest.command('fsdrivertest readonly /tmp/mod/hello', 'fsdrivertest: PASS: readonly')
            guest.command('umount /tmp/mod')
        finally:
            guest.close()
    for path, value in unchanged.items():
        assert digest(path) == value, f'plugin installation changed {path}'
    print(f'ELF plugin ({args.cpus} CPUs): independent SDK build, runtime discovery, aliases, '
          'options, native commands/persistence/identity/bounds, source writes, lifecycle, startup progress/timeout, invalid ABI/ELF and unchanged kernel/bootstrap passed')


if __name__ == '__main__':
    main()
