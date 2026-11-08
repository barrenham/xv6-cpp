#!/usr/bin/env python3
"""Control the running development QEMU's reserved PCIe slots over local QMP."""
import argparse
import json
from pathlib import Path
import re
import socket
import sys
import time


class Qmp:
    def __init__(self, path, timeout=15):
        self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.socket.settimeout(timeout)
        try:
            self.socket.connect(str(path))
            self.stream = self.socket.makefile('rwb', buffering=0)
            self._read()
            self.call('qmp_capabilities')
        except BaseException:
            self.socket.close()
            raise

    def _read(self):
        line = self.stream.readline()
        if not line:
            raise RuntimeError('QEMU closed the QMP connection')
        return json.loads(line)

    def call(self, operation, arguments=None):
        self.stream.write((json.dumps({'execute': operation, 'arguments': arguments or {}}) + '\n').encode())
        while True:
            result = self._read()
            if 'error' in result:
                raise RuntimeError(result['error'].get('desc', str(result['error'])))
            if 'return' in result:
                return result['return']

    def close(self):
        self.stream.close()
        self.socket.close()

    def contains(self, device):
        def walk(value):
            if isinstance(value, dict):
                return value.get('qdev_id') == device or any(walk(v) for v in value.values())
            return isinstance(value, list) and any(walk(v) for v in value)
        return walk(self.call('query-pci'))

    def remove(self, device, timeout=15):
        self.call('device_del', {'id': device})
        deadline = time.monotonic() + timeout
        while self.contains(device):
            if time.monotonic() >= deadline:
                raise RuntimeError('removal is still pending; inspect drvctl hotplug and drvctl events in the guest')
            time.sleep(.05)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--socket', type=Path, default=Path(__file__).resolve().parents[1] / 'build/dev/qemu.qmp')
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('list')
    add = sub.add_parser('add-e1000')
    add.add_argument('id')
    add.add_argument('--slot', default='hotplug0')
    add.add_argument('--net', help='IPv4 network, e.g. 10.0.3.0/24')
    add.add_argument('--mac')
    remove = sub.add_parser('remove')
    remove.add_argument('id')
    remove.add_argument('--timeout', type=float, default=15)
    args = parser.parse_args()
    if args.command != 'list' and not re.fullmatch(r'[A-Za-z0-9_-]{1,48}', args.id):
        parser.error('device id must contain 1..48 letters, digits, underscores or hyphens')
    qmp = Qmp(args.socket)
    try:
        if args.command == 'list':
            print(json.dumps(qmp.call('query-pci'), indent=2))
        elif args.command == 'add-e1000':
            match = re.fullmatch(r'hotplug([0-2])', args.slot)
            slot = int(match[1]) if match else 0
            network = args.net or f'10.0.{slot + 3}.0/24'
            backend = args.id + '-net'
            qmp.call('netdev_add', {'type': 'user', 'id': backend, 'net': network})
            try:
                qmp.call('device_add', {'driver': 'e1000', 'id': args.id, 'bus': args.slot,
                                       'addr': '0.0', 'netdev': backend,
                                       'mac': args.mac or f'52:54:00:48:50:{slot + 16:02x}'})
            except BaseException:
                qmp.call('netdev_del', {'id': backend})
                raise
            print(f'Added {args.id} to {args.slot}; the guest discovers and binds it automatically.')
        else:
            if args.timeout <= 0:
                parser.error('--timeout must be positive')
            qmp.remove(args.id, args.timeout)
            try:
                qmp.call('netdev_del', {'id': args.id + '-net'})
            except RuntimeError:
                pass  # A device supplied outside this tool may use another backend.
            print(f'Removed {args.id}; the guest completed unplug.')
    finally:
        qmp.close()


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, ValueError) as error:
        print(f'hotplug: {error}', file=sys.stderr)
        sys.exit(1)
