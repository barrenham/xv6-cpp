#!/usr/bin/env python3
"""Real PCIe insertion/ejection, driver teardown, network use and slot reuse."""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

from console_login import login_root
from fs_elf_plugins import Guest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from device_hotplug import Qmp


def plain(text):
    return re.sub(r'\x1b\[[0-9;]*m', '', text)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--qemu', required=True)
    for name in ('kernel', 'bootstrap', 'image'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--cpus', type=int, required=True)
    parser.add_argument('--cycles', type=int, default=24)
    parser.add_argument('--boot-network', action='store_true')
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error('--cycles must be positive')
    records = []
    with tempfile.TemporaryDirectory(prefix='xv-device-hotplug-') as temporary:
        directory = Path(temporary)
        image = directory / 'root.img'
        shutil.copyfile(args.image, image)
        socket = directory / 'qmp.sock'
        guest = Guest(args, image, args.kernel.parent / f'device-hotplug-{args.cpus}cpu.log', network=args.boot_network,
                      extra=('-qmp', f'unix:{socket},server=on,wait=off',
                             '-device', 'pcie-root-port,id=hotplug0,bus=pcie.0,addr=10.0,chassis=1',
                             '-device', 'pcie-root-port,id=hotplug1,bus=pcie.0,addr=11.0,chassis=2'))
        qmp = None
        try:
            login_root(guest.expect, guest.process)
            qmp = Qmp(socket)
            def command(text, **kwargs):
                return plain(guest.command(text, **kwargs))
            def status(port=0):
                body = command('drvctl hotplug')
                found = re.search(rf'^\s*(\d+)\s+pcie-slot\.{port}\s+(\d+)\s+(yes|no)\s+(\S+)\s+(\S+)\s+(\S+)', body, re.M)
                assert found, body
                return {'index': int(found[1]), 'generation': int(found[2]), 'present': found[3],
                        'state': found[4], 'interface': found[5], 'removal': found[6]}
            def wait_status(present, port=0):
                deadline = time.monotonic() + 15
                while True:
                    result = status(port)
                    if result['present'] == ('yes' if present else 'no') and result['state'] == ('BOUND' if present else 'UNBOUND'):
                        return result
                    assert time.monotonic() < deadline, (result, guest.output[-2000:])
                    time.sleep(.05)
            def memory():
                result = re.search(r'^MemFree:\s*(\d+)', command('cat /proc/meminfo'), re.M)
                assert result
                return int(result[1])
            first = status()
            interface = 'eth1' if args.boot_network else 'eth0'
            bound = 2 if args.boot_network else 1
            if args.boot_network:
                command('netctl dhcp wait eth0 10', expect='DHCP BOUND')
            assert first['present'] == 'no'
            raw = command('drvctl --raw devices')
            root = re.search(r'^(\d+) virtio-mmio\.[^\n]*device=2', raw, re.M)
            assert root, raw
            rejected = command('drvctl remove ' + root[1], failure=None)
            assert 'remove rejected' in rejected and 'protected' in rejected, rejected
            command('echo ROOT_ALIVE > /root/hotplug-check')
            qmp.call('netdev_add', {'type': 'user', 'id': 'hot-net', 'net': '10.0.3.0/24', 'dhcpstart': '10.0.3.100'})
            baseline = None
            for cycle in range(args.cycles):
                # Alternate two physical identities: slirp retains offered leases and
                # has a bounded DHCP client table, independently of guest teardown.
                mac = f'52:54:00:12:ab:{cycle % 2:02x}'
                qmp.call('device_add', {'driver': 'e1000', 'id': 'hot-nic', 'bus': 'hotplug0',
                                        'addr': '0.0', 'netdev': 'hot-net', 'mac': mac})
                added = wait_status(True)  # No drvctl rescan: automatic worker must bind.
                assert added['index'] == first['index'] and added['generation'] == first['generation'] + cycle + 1, added
                assert added['interface'] == interface and added['removal'] == 'supported', added
                if cycle == 0:
                    stale = command(f'drvctl remove {added["index"]} {first["generation"]}', failure=None)
                    assert 'generation changed' in stale, stale
                    assert status()['state'] == 'BOUND'
                link = command('netctl link')
                assert mac in link and interface + ' UP' in link, link
                if cycle < 2 or cycle == args.cycles - 1:
                    lease = command('netctl dhcp show ' + interface)
                    if 'DHCP OFF' in lease:
                        command('netctl dhcp start ' + interface)
                    command('netctl dhcp wait ' + interface + ' 10', expect='DHCP BOUND')
                    command('ping -q -c 2 -i .01 -W 1 10.0.3.2', expect='2 packets transmitted, 2 received')
                    driver = command('drvctl --raw list')
                    match = re.search(rf'e1000 bound={bound} transfers=(\d+) interrupts=(\d+)', driver)
                    assert match and int(match[1]) and int(match[2]), driver
                if cycle == 0:
                    qmp.close(); qmp = None
                    subprocess.run([sys.executable, str(ROOT / 'tools/device_hotplug.py'),
                                    '--socket', str(socket), 'remove', 'hot-nic'],
                                   check=True, capture_output=True)
                    qmp = Qmp(socket)
                else:
                    qmp.remove('hot-nic')
                removed = wait_status(False)
                assert removed['index'] == added['index'] and removed['generation'] == added['generation'], removed
                assert interface not in command('netctl link')
                if args.boot_network and cycle in (0, args.cycles - 1):
                    command('ping -q -c 1 -W 1 10.0.2.2', expect='1 packets transmitted, 1 received')
                assert '10.0.3.' not in command('netctl route'), 'stale address/lease route'
                command('cat /root/hotplug-check', expect='ROOT_ALIVE')
                free = memory()
                if baseline is None:
                    baseline = free
                assert free >= baseline - 128, (cycle, baseline, free)
                records.append({'cycle': cycle, 'index': added['index'], 'generation': added['generation'], 'free_kib': free})
                print(json.dumps(records[-1]), flush=True)
            # The second port shares its INTx line with the boot NIC. Guest-side
            # safe removal must leave the other image instance and its DMA alive.
            qmp.close(); qmp = None
            subprocess.run([sys.executable, str(ROOT / 'tools/device_hotplug.py'),
                            '--socket', str(socket), 'add-e1000', 'second-nic',
                            '--slot', 'hotplug1', '--net', '10.0.3.0/24',
                            '--mac', '52:54:00:12:cd:77'], check=True, capture_output=True)
            qmp = Qmp(socket)
            second = wait_status(True, 1)
            assert second['interface'] == interface
            command('netctl dhcp wait ' + interface + ' 10', expect='DHCP BOUND')
            command('ping -q -c 1 -W 1 10.0.3.2', expect='1 packets transmitted, 1 received')
            command('drvctl remove ' + str(second['index']), expect='removed')
            assert not qmp.contains('second-nic')
            qmp.call('netdev_del', {'id': 'second-nic-net'})
            wait_status(False, 1)
            if args.boot_network:
                command('ping -q -c 1 -W 1 10.0.2.2', expect='1 packets transmitted, 1 received')
            events = command('drvctl events')
            rows = re.findall(r'^\s*(\d+)\s+(ADDED|BOUND|REMOVED|REJECTED|FAILED)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\S+)', events, re.M)
            assert rows and len(rows) <= 64, events
            own = [row for row in rows if int(row[2]) == first['index']]
            assert [row[1] for row in own[-3:]] == ['ADDED', 'BOUND', 'REMOVED'], events
            assert len({int(row[0]) for row in rows}) == len(rows), events
            if args.cycles >= 24:
                assert int(rows[0][4]) > 0, events
            assert not re.search(r'^\s*\d+\s+(ADDED|BOUND|REMOVED)', command('drvctl events ' + rows[-1][0]), re.M)
            command('drvctl rescan', expect='rescan complete')
            command('sync')
        finally:
            if qmp:
                qmp.close()
            guest.close()
        subprocess.run(['e2fsck', '-fn', str(image)], check=True, capture_output=True)
    (args.kernel.parent / f'device-hotplug-{args.cpus}cpu.json').write_text(json.dumps(records, indent=2) + '\n')
    print('device hotplug: automatic attach, DMA/IRQ teardown, network cleanup, reinsertion and bounded events PASS')


if __name__ == '__main__':
    main()
