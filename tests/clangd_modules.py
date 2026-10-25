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
source = root / 'kernel/memory/paging/vm.cpp'
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
    ]:
        namespace = 'sync' if type_name == 'SpinLock' else 'memory'
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
    opened = []
    for relative in ['tests/allocator_contract.cpp', 'tests/allocator_test_platform.cpp',
                     'kernel/memory/address/physical_address.cppm', 'kernel/memory/allocator/allocator_service.cpp',
                     'tests/sync_contract.cpp', 'tests/sync_test_platform.cpp',
                     'kernel/sync/lock.cppm', 'kernel/sync/spin/spinlock.cppm',
                     'kernel/sync/sleep/sleeplock.cpp', 'kernel/fs/pipe.cpp']:
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
