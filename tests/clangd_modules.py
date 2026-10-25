#!/usr/bin/env python3
"""Check cross-module diagnostics, hover, definition and completion via real LSP.

Build the dev preset first. Reads CMake-built BMIs with the module builder off.
Uses unsaved editor buffers only; never changes project source files.
Background indexing is disabled so the probe only inspects opened files/BMIs and
can shut down without indexing the entire kernel and native standard library.
"""
import argparse
import json
from pathlib import Path
import queue
import subprocess
import tempfile
import threading
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--clangd', required=True)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
source = root / 'kernel/process/lifecycle/process_manager.cpp'
uri = source.as_uri()
original = source.read_text()
messages = queue.Queue()
errors = tempfile.TemporaryFile(mode='w+b')
process = subprocess.Popen(
    [args.clangd, '--background-index=false', '--log=error',
     '--query-driver=' + str(Path(args.clangd).resolve().parent / 'clang++')],
    cwd=root, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=errors,
)


def read_messages():
    try:
        while True:
            headers = {}
            while True:
                line = process.stdout.readline()
                if not line:
                    raise EOFError('clangd closed its output')
                if line in (b'\r\n', b'\n'):
                    break
                key, value = line.decode().split(':', 1)
                headers[key.lower()] = value.strip()
            length = int(headers['content-length'])
            data = bytearray()
            while len(data) < length:
                chunk = process.stdout.read(length - len(data))
                if not chunk:
                    raise EOFError('incomplete LSP message')
                data.extend(chunk)
            messages.put(json.loads(data))
    except Exception as error:
        messages.put(error)


threading.Thread(target=read_messages, daemon=True).start()
request_id = 0


def send(message):
    data = json.dumps(message).encode()
    process.stdin.write(f'Content-Length: {len(data)}\r\n\r\n'.encode() + data)
    process.stdin.flush()


def receive_until(predicate, timeout=25):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        message = messages.get(timeout=max(0.01, deadline - time.monotonic()))
        if isinstance(message, Exception):
            raise message
        if 'method' in message and 'id' in message:
            # Acknowledge optional server requests such as progress creation.
            send({'jsonrpc': '2.0', 'id': message['id'], 'result': None})
        if predicate(message):
            return message
    raise TimeoutError('LSP response timed out')


def notify(method, params):
    send({'jsonrpc': '2.0', 'method': method, 'params': params})


def request(method, params):
    global request_id
    request_id += 1
    current = request_id
    send({'jsonrpc': '2.0', 'id': current, 'method': method, 'params': params})
    response = receive_until(lambda message: message.get('id') == current)
    if 'error' in response:
        raise RuntimeError(response['error'])
    return response.get('result')


def position(text, offset):
    prefix = text[:offset]
    # Protocol positions are UTF-16 code units, not bytes or Python code points.
    return {'line': prefix.count('\n'),
            'character': len(prefix.rsplit('\n', 1)[-1].encode('utf-16-le')) // 2}


def at(text, token):
    return {'textDocument': {'uri': uri},
            'position': position(text, text.rindex(token))}


try:
    request('initialize', {
        'processId': None,
        'rootUri': root.as_uri(),
        'workspaceFolders': [{'uri': root.as_uri(), 'name': root.name}],
        'capabilities': {'textDocument': {'definition': {'linkSupport': True}}},
    })
    notify('initialized', {})
    notify('textDocument/didOpen', {
        'textDocument': {'uri': uri, 'languageId': 'cpp', 'version': 1, 'text': original},
    })
    diagnostic = receive_until(lambda message:
        message.get('method') == 'textDocument/publishDiagnostics'
        and message.get('params', {}).get('uri') == uri)
    problems = diagnostic['params'].get('diagnostics', [])
    assert not [item for item in problems if item.get('severity') == 1], problems
    print('module consumer diagnostics: no errors', flush=True)

    probe = original + '''
void xv_clangd_module_probe()
{
  xv::memory::PhysicalPageAllocator &memory = xv::memory::allocator();
  memory.max_order();
}
'''
    notify('textDocument/didChange', {
        'textDocument': {'uri': uri, 'version': 2}, 'contentChanges': [{'text': probe}],
    })
    hover = request('textDocument/hover', at(probe, 'PhysicalPageAllocator'))
    assert hover and 'PhysicalPageAllocator' in json.dumps(hover), hover
    print('cross-module type hover: passed', flush=True)

    definitions = request('textDocument/definition', at(probe, 'PhysicalPageAllocator'))
    assert definitions, 'no module definition returned'
    if not isinstance(definitions, list):
        definitions = [definitions]
    targets = [item.get('targetUri', item.get('uri', '')) for item in definitions]
    assert any(target.endswith('/page_allocator.cppm') for target in targets), targets
    print('cross-module definition: page_allocator.cppm', flush=True)

    completion_text = probe.replace('  memory.max_order();', '  memory.')
    notify('textDocument/didChange', {
        'textDocument': {'uri': uri, 'version': 3},
        'contentChanges': [{'text': completion_text}],
    })
    offset = completion_text.rindex('memory.') + len('memory.')
    completion = request('textDocument/completion', {
        'textDocument': {'uri': uri}, 'position': position(completion_text, offset),
        'context': {'triggerKind': 2, 'triggerCharacter': '.'},
    })
    items = completion.get('items', []) if isinstance(completion, dict) else completion
    labels = [item.get('filterText', item.get('label', '')).strip() for item in items or []]
    for name in ['allocate', 'deallocate', 'split_to_pages', 'supports', 'max_order']:
        assert any(name == label or label.startswith(name + '(') for label in labels), (name, labels)
    print('cross-module member completion: passed', flush=True)
    for version, type_name, module_file, methods in [
        (4, 'PhysicalAddress', 'physical_address.cppm',
         ['raw', 'page_number', 'is_aligned', 'align_up', 'align_down', 'advanced']),
        (5, 'PhysicalMemoryRange', 'physical_memory_range.cppm',
         ['begin', 'end', 'contains', 'contains_block', 'page_index', 'page_address']),
        (6, 'SpinLock', 'spinlock.cppm',
         ['init', 'acquire', 'release', 'holding', 'kind', 'name']),
        (7, 'AddressSpace', 'address_space.cppm',
         ['size', 'copy_in', 'copy_out', 'copy_string', 'resize', 'handle_fault']),
        (8, 'DescriptorTable', 'process_resources.cppm',
         ['get', 'install', 'take', 'close', 'duplicate', 'close_all']),
        (9, 'WorkingDirectory', 'process_resources.cppm',
         ['get', 'duplicate', 'replace', 'clear']),
        (10, 'Inode', 'inode.cppm', ['kind', 'device_major', 'device', 'number', 'size_bytes']),
        (11, 'Buffer', 'block_cache.cppm', ['bytes', 'block_number', 'device']),
    ]:
        namespace = ('sync' if type_name == 'SpinLock' else
                     'fs' if type_name in ['Inode', 'Buffer'] else
                     'process' if type_name in ['AddressSpace', 'DescriptorTable', 'WorkingDirectory']
                     else 'memory')
        value_probe = original + (
            '\nvoid xv_clangd_value_probe()\n{\n'
            f'  xv::{namespace}::{type_name} value;\n  value.\n}}\n'
        )
        notify('textDocument/didChange', {
            'textDocument': {'uri': uri, 'version': version},
            'contentChanges': [{'text': value_probe}],
        })
        definitions = request('textDocument/definition', at(value_probe, type_name))
        if not isinstance(definitions, list):
            definitions = [definitions] if definitions else []
        targets = [item.get('targetUri', item.get('uri', '')) for item in definitions]
        assert any(target.endswith('/' + module_file) for target in targets), targets
        offset = value_probe.rindex('value.') + len('value.')
        completion = request('textDocument/completion', {
            'textDocument': {'uri': uri}, 'position': position(value_probe, offset),
            'context': {'triggerKind': 2, 'triggerCharacter': '.'},
        })
        items = completion.get('items', []) if isinstance(completion, dict) else completion
        labels = [item.get('filterText', item.get('label', '')).strip() for item in items or []]
        for name in methods:
            assert any(name == label or label.startswith(name + '(') for label in labels), (name, labels)
        print(f'{type_name} definition and member completion: passed', flush=True)
    memory_uri = uri
    driver_file = root / 'kernel/boot/main.cpp'
    driver_text = driver_file.read_text()
    uri = driver_file.as_uri()
    notify('textDocument/didOpen', {
        'textDocument': {'uri': uri, 'languageId': 'cpp', 'version': 1,
                         'text': driver_text},
    })
    diagnostic = receive_until(lambda message:
        message.get('method') == 'textDocument/publishDiagnostics'
        and message.get('params', {}).get('uri') == uri)
    problems = diagnostic['params'].get('diagnostics', [])
    assert not [item for item in problems if item.get('severity') == 1], problems
    for version, type_name, service, module_file, methods in [
        (2, 'SerialPort', 'serial_port', 'serial_port.cppm',
         ['attach', 'write', 'put_sync', 'handle_interrupt', 'initialize', 'irq']),
        (3, 'BlockDevice', 'block_device', 'block_device.cppm',
         ['transfer', 'handle_interrupt', 'initialize', 'irq']),
        (4, 'InterruptController', 'interrupt_controller', 'interrupt_controller.cppm',
         ['initialize_hart', 'enable', 'claim', 'complete', 'set_priority']),
        (5, 'Process', 'current', 'process.cppm',
         ['id', 'registers', 'memory', 'files', 'directory', 'is_killed']),
        (6, 'Scheduler', 'scheduler', 'scheduler.cppm', ['name', 'run', 'yield', 'suspend']),
        (7, 'ExecutableLoader', 'executable_loader', 'executable_loader.cppm', ['name', 'load']),
        (8, 'BlockCache', 'cache', 'block_cache.cppm', ['read', 'write', 'pin', 'unpin', 'release']),
        (9, 'Journal', 'journal', 'journal.cppm', ['begin', 'end', 'record', 'sync']),
        (10, 'FileSystem', 'filesystem', 'filesystem.cppm', ['lookup', 'create', 'link', 'unlink', 'read', 'write']),
        (11, 'OpenFileTable', 'files', 'open_file.cppm', ['allocate', 'retain', 'release', 'read', 'write', 'stat']),
        (12, 'DeviceRegistry', 'devices', 'open_file.cppm', ['attach', 'get']),
        (13, 'TrapController', 'controller', 'trap_controller.cppm',
         ['initialize_hart', 'handle_user', 'handle_kernel', 'prepare_return', 'return_to_user']),
        (14, 'TickClock', 'clock', 'tick_clock.cppm', ['initialize', 'interrupt', 'uptime', 'pause']),
        (15, 'KernelPageTable', 'kernel_page_table', 'kernel_page_table.cppm',
         ['initialize', 'activate_hart', 'root']),
    ]:
        namespace = ('memory' if type_name == 'KernelPageTable' else
                     'trap' if type_name in ['TrapController', 'TickClock'] else
                     'process' if type_name in ['Process', 'Scheduler', 'ExecutableLoader'] else
                     'fs' if type_name in ['BlockCache', 'Journal', 'FileSystem', 'OpenFileTable', 'DeviceRegistry']
                     else 'drivers')
        initializer = f'xv::{namespace}::{service}()'
        if type_name == 'Process':
            initializer = '*' + initializer
        probe = driver_text + (
            '\nvoid xv_clangd_driver_probe()\n{\n'
            f'  xv::{namespace}::{type_name} &device = {initializer};\n'
            '  device.\n}\n'
        )
        notify('textDocument/didChange', {
            'textDocument': {'uri': uri, 'version': version},
            'contentChanges': [{'text': probe}],
        })
        hover = request('textDocument/hover', at(probe, type_name))
        assert hover and type_name in json.dumps(hover), hover
        definitions = request('textDocument/definition', at(probe, type_name))
        if not isinstance(definitions, list):
            definitions = [definitions] if definitions else []
        targets = [item.get('targetUri', item.get('uri', '')) for item in definitions]
        assert any(target.endswith('/' + module_file) for target in targets), targets
        offset = probe.rindex('device.') + len('device.')
        completion = request('textDocument/completion', {
            'textDocument': {'uri': uri}, 'position': position(probe, offset),
            'context': {'triggerKind': 2, 'triggerCharacter': '.'},
        })
        items = completion.get('items', []) if isinstance(completion, dict) else completion
        labels = [item.get('filterText', item.get('label', '')).strip() for item in items or []]
        for name in methods:
            assert any(name == label or label.startswith(name + '(') for label in labels), (name, labels)
        print(f'{type_name} hover, definition and member completion: passed', flush=True)
    notify('textDocument/didClose', {'textDocument': {'uri': uri}})
    uri = memory_uri
    opened = []
    for relative in ['tests/allocator_contract.cpp', 'tests/allocator_test_platform.cpp',
                     'kernel/memory/address/physical_address.cppm', 'kernel/memory/allocator/allocator_service.cpp',
                     'tests/sync_contract.cpp', 'tests/sync_test_platform.cpp',
                     'kernel/sync/lock.cppm', 'kernel/sync/spin/spinlock.cppm',
                     'kernel/sync/sleep/sleeplock.cpp', 'kernel/fs/pipe/pipe.cpp',
                     'tests/driver_test_platform.cpp', 'tests/driver_contract.cpp',
                     'kernel/drivers/serial/uart16550.cppm',
                     'kernel/drivers/console/line_console.cpp',
                     'kernel/drivers/block/virtio/virtio_block.cpp',
                     'kernel/drivers/board/qemu_virt.cpp',
                     'tests/process_contract.cpp',
                     'kernel/process/process.cppm',
                     'kernel/process/resources/process_resources.cpp',
                     'kernel/process/memory/address_space.cpp',
                     'kernel/process/scheduler/policies/round_robin.cpp',
                     'kernel/process/scheduler/runtime/scheduler_context.cppm',
                     'kernel/process/scheduler/runtime/scheduler_context.cpp',
                     'kernel/process/arch/cpu.cppm', 'kernel/process/arch/cpu.cpp',
                     'kernel/process/lifecycle/first_return.cpp',
                     'kernel/process/wait/sleep.cpp',
                     'kernel/process/lifecycle/process_manager.cppm',
                     'kernel/process/storage/process_store.cppm',
                     'kernel/process/storage/paged_process_store.cpp',
                     'kernel/process/scheduler/queues/fifo_ready_queue.cpp',
                     'kernel/process/storage/process_store.cpp',
                     'kernel/process/scheduler/queues/ready_queue.cpp',
                     'kernel/process/wait/wait_channels.cpp',
                     'kernel/process/arch/cpu_affinity.cpp',
                     'tests/process_storage_checks.cpp',
                     'tests/process_storage_test_platform.cpp',
                     'tests/process_storage_contract.cpp',
                     'kernel/process/storage/kernel_platform.cpp',
                     'kernel/process/exec/elf_loader.cpp',
                     'tests/fs_contract.cpp', 'tests/fs_test_platform.cpp',
                     'kernel/fs/fs.cppm', 'kernel/fs/cache/lru_block_cache.cpp',
                     'kernel/fs/journal/journal.cpp', 'kernel/fs/inode/xv6_filesystem.cpp',
                     'kernel/fs/path/path_operations.cpp', 'kernel/fs/file/file_operations.cpp',
                     'kernel/syscall/syscall.cppm',
                     'kernel/syscall/arguments.cppm', 'kernel/syscall/handlers.cppm',
                     'kernel/syscall/syscall.cpp', 'kernel/syscall/arguments.cpp',
                     'kernel/syscall/sysfile.cpp', 'kernel/syscall/sysproc.cpp',
                     'kernel/syscall/sysvm.cpp',
                     'tests/trap_contract.cpp', 'tests/trap_test_platform.cpp',
                     'kernel/trap/trap.cppm', 'kernel/trap/event/trap_event.cppm',
                     'kernel/trap/controller/trap_controller.cpp',
                     'kernel/trap/handlers/system_call.cpp',
                     'kernel/trap/clock/tick_clock.cpp',
                     'kernel/memory/paging/kernel_page_table.cppm',
                     'kernel/memory/paging/kernel_page_table.cpp',
                     'kernel/support/print.cppm', 'kernel/support/printk.cpp',
                     'kernel/support/runtime.cppm', 'kernel/support/string.cpp',
                     'kernel/support/compiler_memory.cpp',
                     'kernel/arch/arch.cppm', 'kernel/sync/interrupts.cppm',
                     'kernel/sync/platform/kernel_bridge.cpp',
                     'kernel/sync/platform/kernel_platform.cpp',
                     'kernel/memory/paging/user_paging.cppm',
                     'kernel/process/resources/resource_platform.cppm',
                     'kernel/process/resources/kernel_platform.cpp',
                     'tests/process_test_platform.cpp', 'tests/runtime_contract.cpp',
                     'kernel/boot/boot.cppm', 'kernel/boot/state.cppm',
                     'kernel/boot/start.cpp', 'kernel/boot/entry.cpp',
                     'kernel/memory/boot/kalloc.cpp',
                     'kernel/support/stats.cpp',
                     'kernel/trap/arch/riscv/entry.cpp',
                     'kernel/trap/arch/riscv/kernel_platform.cpp']:
        file = root / relative
        target_uri = file.as_uri()
        notify('textDocument/didOpen', {
            'textDocument': {'uri': target_uri, 'languageId': 'cpp', 'version': 1,
                             'text': file.read_text()},
        })
        diagnostic = receive_until(lambda message:
            message.get('method') == 'textDocument/publishDiagnostics'
            and message.get('params', {}).get('uri') == target_uri)
        problems = diagnostic['params'].get('diagnostics', [])
        assert not [item for item in problems if item.get('severity') == 1], (relative, problems)
        print(relative + ': prebuilt modules, no errors', flush=True)
        if relative == 'kernel/trap/arch/riscv/kernel_platform.cpp':
            target_text = file.read_text()
            offset = target_text.index('dispatch();', target_text.index('xv::syscall::'))
            location = {'textDocument': {'uri': target_uri},
                        'position': position(target_text, offset)}
            hover = request('textDocument/hover', location)
            assert hover and 'dispatch' in json.dumps(hover), hover
            definitions = request('textDocument/definition', location)
            assert definitions, 'no syscall module definition returned'
            if not isinstance(definitions, list):
                definitions = [definitions]
            targets = [item.get('targetUri', item.get('uri', ''))
                       for item in definitions]
            assert any(target.endswith(('/syscall.cppm', '/syscall.cpp'))
                       for target in targets), targets
            print('syscall dispatch hover and module definition: passed', flush=True)

        opened.append(target_uri)
    for target_uri in opened:
        notify('textDocument/didClose', {'textDocument': {'uri': target_uri}})
    notify('textDocument/didClose', {'textDocument': {'uri': uri}})
    request('shutdown', None)
    notify('exit', None)
    process.wait(timeout=5)
    assert process.returncode == 0, process.returncode
except Exception:
    errors.seek(0)
    print(errors.read().decode(errors='replace')[-6000:])
    raise
finally:
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
    errors.close()
